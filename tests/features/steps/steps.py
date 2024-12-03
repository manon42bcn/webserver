
import requests
import string
import json
import random
import os
from behave import step
from bs4 import BeautifulSoup
import http.client
import warnings
from urllib3.exceptions import NotOpenSSLWarning

def map_table(table):
    map_table = {}
    if table:
        for row in table:
            map_table[row['param_name']] = row['value']
    return map_table

def generate_chunks(file_path, chunk_size=1024):
    with open(file_path, "rb") as f:
        while chunk := f.read(chunk_size):
            yield chunk

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
def assert_element_content_by_label(context, label, content):
    find_elements = context.soup.find_all(label)
    found = False
    for elements in find_elements:
        if elements.text == content:
            context.logger.debug(f"Element has been found.: {elements.text}")
            found = True
            break
    assert found, f"Element {label} with content {content} has been not found."

