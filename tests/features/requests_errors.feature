Feature: Request Error Cases

    Scenario Outline: Request Without method and using host to the same port gets bad request error
        Given set connection and headers for ip "127.0.0.1" port "8181" and domain "<host>"
        And send a "'" request to "/" using set up domain and headers and status code "400"
        When I parse html response body
        Then The response body content includes "h2" with content "400 - Bad Request"

        Examples:
            | host     |
            | one.com  |
            | two.com  |
            | tree.com |

    Scenario Outline: Request using fake method and using host to the same port gets bad request error
        Given set connection and headers for ip "127.0.0.1" port "8181" and domain "<host>"
        And send a "FAKE" request to "/" using set up domain and headers and status code "400"
        When I parse html response body
        Then The response body content includes "h2" with content "400 - Bad Request"

        Examples:
            | host     |
            | one.com  |
            | two.com  |
            | tree.com |

    Scenario Outline: Request using fake method and using different ports gets bad request error
        Given set connection and headers for ip "127.0.0.1" port "<port>" and domain "localhost"
        And send a "FAKE" request to "/" using set up domain and headers and status code "400"
        When I parse html response body
        Then The response body content includes "h2" with content "400 - Bad Request"

        Examples:
            | port  |
            | 8080  |
            | 8081  |
            | 9090  |
