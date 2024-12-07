Feature: Request Error Cases

    Scenario Outline: Request for a non existing resource and get a 404 response
        Given send a request to "http://localhost:<port>/basic_request/fake_resource" and get code status "404"
        When I parse html response body
        Then The response body content includes "<label>" with content "<error_msg>"

        Examples:
            | port | label | error_msg                  |
            | 8080 | h1    | Oops! Something went wrong |
            | 8081 | h2    | 404 - Not Found            |
            | 9090 | h2    | 404 - Not Found            |

    Scenario Outline: URI too long error is handled correctly using host to the same port
        Given set connection and headers for ip "127.0.0.1" port "8181" and domain "<host>"
        And send a "GET" request to "[TOO_LONG_URL]" using set up domain and headers and status code "414"
        When I parse html response body
        Then The response body content includes "h2" with content "414 - URI Too Long"

        Examples:
            | host     |
            | one.com  |
            | two.com  |
            | tree.com |

    Scenario Outline: URI too long error is handled correctly using different ports
        Given set connection and headers for ip "127.0.0.1" port "<port>" and domain "localhost"
        And send a "GET" request to "[TOO_LONG_URL]" using set up domain and headers and status code "414"
        When I parse html response body
        Then The response body content includes "h2" with content "414 - URI Too Long"

        Examples:
            | port  |
            | 8080  |
            | 8081  |
            | 9090  |


