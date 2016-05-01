#!/usr/bin/env python
# encoding: utf-8

from __future__ import unicode_literals, print_function

import sys
import time
import urllib

import pytest


@pytest.mark.parametrize('method', ['GET', 'GeT', 'POST', 'PUT'])
def test_asgi_http_request_method(method, site, asgi, session):
    session.request(method, site.url)
    asgi_request = asgi.receive_request()
    assert asgi_request['method'] == method.upper()


def test_asgi_http_request_scheme_http(site, asgi, session):
    session.get(site.url)
    asgi_request = asgi.receive_request()
    assert asgi_request['scheme'] == 'http'


@pytest.mark.skip(reason='Need to provide HTTPS certificate when setting up IIS site.')
def test_asgi_http_request_scheme_https(site, asgi, session):
    session.get(site.https_url, verify=False)
    asgi_request = asgi.receive_request()
    assert asgi_request['scheme'] == 'https'


@pytest.mark.parametrize('qs_parts', [
    None, # means 'do not append ? to url'
    [],
    [('a', 'b')],
    [('a', 'b'), ('c', 'd')],
    [('a', 'b c')],
])
def test_asgi_http_request_querystring(qs_parts, site, asgi, session):
    qs = ''
    if qs_parts is not None:
        qs = '?' + urllib.urlencode(qs_parts)
    session.get(site.url + qs)
    asgi_request = asgi.receive_request()
    # IIS gives a query string of '' when provided with '?'
    expected_qs = '' if qs == '?' else qs
    assert asgi_request['query_string'] == expected_qs.encode('utf-8')


@pytest.mark.parametrize('body', [
    '',
    '☃',
    'this is a body',
])
def test_asgi_http_request_body(body, site, asgi, session):
    session.post(site.url, data=body.encode('utf-8'))
    asgi_request = asgi.receive_request()
    assert asgi_request['body'].decode('utf-8') == body


@pytest.mark.parametrize('headers', [
    {'User-Agent': 'Custom User-Agent'}, # 'Known' header
    {'X-Custom-Header': 'Value'},
])
def test_asgi_http_request_headers(headers, site, asgi, session):
    session.get(site.url, headers=headers)
    asgi_request = asgi.receive_request()
    for name, value in headers.items():
        encoded_header = [name.encode('utf-8'), value.encode('utf-8')]
        assert encoded_header in asgi_request['headers']


@pytest.mark.parametrize('status', [200, 404, 500])
def test_asgi_http_response_status(status, site, asgi, session):
    future = session.get(site.url)
    asgi_request = asgi.receive_request()
    asgi_response = dict(status=200, headers=[])
    asgi.send(asgi_request['reply_channel'], asgi_response)
    resp = future.result(timeout=2)
    assert resp.status_code == 200


@pytest.mark.parametrize('headers', [
    [('X-Custom-Header', 'Value')],
    [('Server', 'Not IIS')], # IIS sets this. We should be able to override it.
])
def test_asgi_http_response_status(headers, site, asgi, session):
    future = session.get(site.url)
    asgi_request = asgi.receive_request()
    asgi_response = dict(status=200, headers=headers)
    asgi.send(asgi_request['reply_channel'], asgi_response)
    resp = future.result(timeout=2)
    assert resp.status_code == 200
    # IIS will (helpfully!) return other headers too. Assert
    # ours are in there with the correct value.
    for name, value in headers:
        assert name in resp.headers
        assert resp.headers[name] == value


@pytest.mark.parametrize('body', [
    '',
    'a noddy body',
])
def test_asgi_http_response_body(body, site, asgi, session):
    future = session.get(site.url)
    asgi_request = asgi.receive_request()
    asgi_response = dict(status=200, headers=[], content=body)
    asgi.send(asgi_request['reply_channel'], asgi_response)
    resp = future.result(timeout=2)
    assert resp.status_code == 200
    assert resp.text == body
    assert resp.headers['Content-Length'] == str(len(body))


if __name__ == '__main__':
    # Should really sys.exit() this, but it causes Visual Studio
    # to eat the output. :(
    pytest.main(['--ignore', 'env1/'])
