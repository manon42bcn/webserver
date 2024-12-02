
import requests
import string
import json
import random
import os
from behave import step
from bs4 import BeautifulSoup
import warnings
from urllib3.exceptions import NotOpenSSLWarning

FILE_PATH = os.path.join(os.path.dirname(__file__), "../../../websrv/data/img/party_static.webp")

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
        files = {"file": open(FILE_PATH, "rb")}
        response = context.session.request(method.upper(), url, files=files)
    if method == "EMPTY":
        response = context.session.request(url)
    else:
        response = context.session.request(method.upper(), url)
    assert response.status_code == int(status_code), f"Wrong status code: {response.status_code}"
    context.html_content = response.text
    context.logger.debug(f"Response Body: {context.html_content}")

@step('send party image to "{url}" with status code "{status_code}"')
@step('send party image to previous domain with status code "{status_code}"')
def send_party_image_to_location(context, status_code, url=None):
    files = {"file": open(FILE_PATH, "rb")}
    if url is None:
        url = context.base_url
    response = requests.post(url, files=files)
    assert response.status_code == int(status_code), f"Wrong status code: {response.status_code}"
    context.html_content = response.text


@step('post data to "{url}" encoded json with status code "{status_code}"')
def post_data_to_location(context, url, status_code):
    params = context.text
    print (params)
    payload = json.loads(params)
    # Ruta del archivo que deseas subir
    # file_path = "/Users/cx03019/Documents/Cursus/webserver/studying/websrv/data/img/party_static.webp"

    # Preparar el archivo para la solicitud
    files = {"file": open(FILE_PATH, "rb")}

    # Enviar la solicitud POST con el archivo
    response = requests.post(url, files=files)
    # print(payload)
    # headers = {
    #     "Content-Type": "application/json"
    # }
    # response = requests.post(url, json=payload, headers=headers)
    context.html_content = response.text
    print(context.html_content)
    # assert response.status_code == int(status_code), f"Wrong status code: {response.status_code}"
    # url = f"{context.base_url}{location}"
    # response = context.session.post(url)
    # assert response.status_code == int(status_code), f"Wrong status code: {response.status_code}"
    # context.html_content = response.text
    # context.logger.debug(f"Response Body: {context.html_content}")


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

