Feature: Perl CGI Request cases

    Scenario Outline: Send a petition to a Perl cgi using path as var
        Given set connection and headers for ip "127.0.0.1" port "<port>" and domain "<host>"
        And send a "GET" request to "/not_home_perl/this/is/part/of/a/path" using set up domain and headers and status code "200"
        When I parse html response body
        Then The response body content includes "div" with content as text
            """
            this
            is
            part
            of
            a
            path
            """

        Examples:
            | host          | port |
            | locahost      | 8080 |
            | example.com   | 8080 |

    Scenario Outline: Send a request using path query to perl cgi script
        Given set connection and headers for ip "127.0.0.1" port "<port>" and domain "<host>"
        And send a "GET" request to "/not_home_perl?var=1&var2=2" using set up domain and headers and status code "200"
        When I parse html response body
        Then The response body content includes "p" with content "Query String: var=1&var2=2"

        Examples:
            | host          | port |
            | locahost      | 8080 |
            | example.com   | 8080 |

    Scenario Outline: Send a request to a endless process perl cgi to get timeout error
        Given set connection and headers for ip "127.0.0.1" port "<port>" and domain "<host>"
        And send a "GET" request to "/home/one/one_pl/this/is/timeout/request" using set up domain and headers and status code "504"
        When I parse html response body
        Then The response body content includes "h2" with content "504 - Gateway Timeout"

        Examples:
            | host          | port |
            | locahost      | 8080 |
            | example.com   | 8080 |

    Scenario Outline: Malformed response from a perl CGI returns a bad gateway error
        Given set connection and headers for ip "127.0.0.1" port "8080" and domain "<host>"
        And send a "GET" request to "/not_home_new_perl/hello" using set up domain and headers and status code "502"
        When I parse html response body
        Then The response body content includes "h2" with content "502 - Bad Gateway"

        Examples:
            | host        | port | request        |
            | example.com | 8080 | /              |
            | locahost    | 8080 | /basic_request |

