
import requests
import http.client
from behave import step
from bs4 import BeautifulSoup
import warnings
from urllib3.exceptions import NotOpenSSLWarning

@step('set connection and headers for ip "{ip}" port "{port}" and domain "{domain}"')
def set_headers_before_request(context, ip, port, domain):
    context.connection = http.client.HTTPConnection(ip, int(port))
    context.headers = {"Host": domain}
    context.logger.debug(f"Connection and headers set up for ip {ip}, port {port} and domain {domain} has been set up.")

@step('send a "{method}" request to "{location}" using set up domain and headers and status code "{status_code}"')
def send_request_using_host(context, method, location, status_code):
    context.connection.request(method, location, headers=context.headers)
    response = context.connection.getresponse()
    assert response.status == int(status_code)
    context.html_content = response.read().decode()
    context.logger.debug(f"Request response: {context.html_content}")

@step('send a request to "{url}" and get code status "{code_status}"')
def send_request_to_url(context, url, code_status):
    response = requests.get(url)
    assert response.status_code == int(code_status), f"Failed to fetch page: {response.status_code}"
    context.html_content = response.text
    context.logger.debug(f"Request response: {context.html_content}")

@step('I parse html response body')
def parse_html_response(context):
    context.soup = BeautifulSoup(context.html_content, 'html.parser')
    context.logger.debug("Content has been parsed.")

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

