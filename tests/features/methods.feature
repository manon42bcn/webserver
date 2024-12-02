Feature: Request using forbidden methods

    Scenario Outline: Request using not allowed method and using host to the same port gets bad request error
        Given set connection and headers for ip "127.0.0.1" port "<port>" and domain "<host>"
        And send a "HEAD" request to "<request>" using set up domain and headers and status code "405"

        Examples:
            | host     | port | request        |
            | one.com  | 8181 | /              |
            | two.com  | 8181 | /              |
            | tree.com | 8181 | /              |
            | locahost | 8080 | /basic_request |
            | locahost | 8081 | /basic_request |
            | locahost | 9090 | /basic_request |

    Scenario: A DELETE request over a path cannot be resolved as resource
        Given set connection and headers for ip "127.0.0.1" port "8081" and domain "localhost"
        And send a "DELETE" request to "/try_delete/" using set up domain and headers and status code "404"
        When I parse html response body
        Then The response body content includes "h2" with content "404 - Not Found"

    Scenario: Send a POST request over a server where is allowed
        Given set connection and headers for ip "127.0.0.1" port "8081" and domain "localhost"
        And send a "DELETE" request to "/try_delete/" using set up domain and headers and status code "404"

    Scenario Outline: Wrong method
        Given set connection and headers for ip "127.0.0.1" port "<port>" and domain "localhost"
        And send a "<method>" request to "/resource" using set up domain and headers and status code "400"
        When I parse html response body
        Then The response body content includes "h2" with content "414 - URI Too Long"

        Examples:
            | port | method |
            | 8080 | DELETE |
            | 8081 | DELETE |
            | 9090 | DELETE |

