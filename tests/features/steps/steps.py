
import requests
import string
import json
import random
import os
import uuid
from behave import step
from bs4 import BeautifulSoup
import warnings
from urllib3.exceptions import NotOpenSSLWarning

def map_table(table):
    map_table = {}
    if table:
        for row in table:
            map_table[row['param_name']] = row['value']
    return map_table

@step('set connection and headers for ip "{ip}" port "{port}" and domain "{domain}"')
def set_headers_before_request(context, ip, port, domain):
    context.session = requests.Session()
    context.base_url = f"http://{ip}:{port}"
    context.session.headers.update({"Host": domain})
    context.logger.debug(f"Session and headers set up for IP {ip}, port {port}, and domain {domain}.")

@step('send a "{method}" request to "{location}" using set up domain and headers and status code "{status_code}"')
def send_request_using_host(context, method, location, status_code):
    if location == "[TOO_LONG_URL]":
        location = ''.join(random.choices(string.ascii_letters, k=2048))
        location = f"/{location}"
    url = f"{context.base_url}{location}"
    if method == "POST":
        params = map_table(context.table)
        files = {}
        for key, value in params.items():
            files[key] = open(os.path.join(os.path.dirname(__file__), f"../../resources/{value}"), "rb")
        response = context.session.request(method.upper(), url, files=files)
    elif method == "POST_EMPTY":
        method = "POST"
        response = context.session.request(method.upper(), url)
    else:
        response = context.session.request(method.upper(), url)
    assert response.status_code == int(status_code), f"Wrong status code: {response.status_code}"
    context.html_content = response.text
    context.logger.debug(f"Response Body: {context.html_content}")

@step('I save html response as "{key}"')
def save_html_response(context, key):
    context.storage[key] = context.html_content

def generate_chunks(file_path, boundary, mimetype="application/octet-stream", chunk_size=1024):
    file_name = file_path.split("/")[-1]

    yield f"--{boundary}\r\n".encode()
    yield f"Content-Disposition: form-data; name=\"file\"; filename=\"{file_name}\"\r\n".encode()
    yield f"Content-Type: {mimetype}\r\n\r\n".encode()

    with open(file_path, "rb") as f:
        while chunk := f.read(chunk_size):
            yield chunk
    yield f"\r\n--{boundary}--\r\n".encode()

@step('send a chunked request to "{location}" using set up domain and headers with status code "{status_code}"')
def send_chunked_file_with_requests(context, location, status_code):
    url = f"{context.base_url}{location}"
    params = map_table(context.table)
    boundary = f"------------------------{uuid.uuid4().hex}"
    headers = {
        "Content-Type": f"multipart/form-data; boundary={boundary}",
        "Transfer-Encoding": "chunked"
    }
    response = context.session.request(
        "POST",
        url,
        data=generate_chunks(os.path.join(os.path.dirname(__file__), f"../../resources/{params['file']}"),
                             boundary,
                             params['mimetype']),
        headers=headers
    )
    assert response.status_code == int(status_code), f"Wrong status code: {response.status_code}"
    context.html_content = response.text

@step('send a request to "{url}" and get code status "{code_status}"')
def send_request_to_url(context, url, code_status):
    response = requests.get(url)
    assert response.status_code == int(code_status), f"Failed to fetch page: {response.status_code}"
    context.html_content = response.text
    context.logger.debug(f"Request response: {context.html_content}")

@step('I parse html response body')
def parse_html_response(context):
    context.soup = BeautifulSoup(context.html_content, 'html.parser')
    context.logger.debug(f"Content has been parsed: {context.soup}")

@step('The response body content includes "{label}" with content "{content}"')
@step('The response body content includes "{label}" with content as text')
def assert_element_content_by_label(context, label, content=None):
    if not content:
        content = context.text
    find_elements = context.soup.find_all(label)
    found = False
    for elements in find_elements:
        context.logger.debug(f"Element: {elements.text}")
        if elements.text.strip() == content.strip():
            context.logger.debug(f"Element has been found.: {elements.text}")
            found = True
            break
    assert found, f"Element {label} with content {content} has been not found."

@step('open "{file_path}" file and save its content in context with key "{key}"')
def open_and_save_content_file(context, file_path, key):
    params = map_table(context.table)
    if params['read_mode'] == 'read':
        mode = 'r'
    elif params['read_mode'] == 'binary':
        mode = 'rb'
    else:
        mode = 'r'
    file_path = os.path.join(os.path.dirname(__file__), f"../../resources/{file_path}")
    with open(file_path, mode) as f:
        context.storage[key] = f.read()
    context.logger.debug("Data from {path} has been save at context.{key}")

@step('the content of "{key1}" and "{key2}" context keys are equal')
def compare_context_keys(context, key1, key2):
    print(f"{context.storage[key1].strip()} \n---\n {context.storage[key2].strip()}")
    assert context.storage[key1].strip() == context.storage[key2].strip(), f"The content of {key1} and {key2} are not equal"
    context.logger.debug(f"Context keys {key1} and {key2} are equal")
