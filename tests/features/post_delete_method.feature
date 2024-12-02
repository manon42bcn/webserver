Feature: Post and Delete methods

    Scenario Outline: Post and Delete methods details posting and deleting a test resource
        Given set connection and headers for ip "127.0.0.1" port "8183" and domain "<host>"
        And send a "POST" request to "/" using set up domain and headers and status code "201"
        Then send a "POST" request to "/" using set up domain and headers and status code "409"
        When I parse html response body
        Then The response body content includes "h2" with content "409 - Conflict"
        When send a "DELETE" request to "/party_static.webp" using set up domain and headers and status code "204"

        Examples:
            | host         |
            | localhost    |
            | fivehost.com |

    Scenario Outline: Post over request limit gives a 413 error
        Given set connection and headers for ip "127.0.0.1" port "8182" and domain "<host>"
        And send a "POST" request to "/" using set up domain and headers and status code "413"
        When I parse html response body
        Then The response body content includes "h2" with content "413 - Payload Too Large"

        Examples:
            | host         |
            | localhost    |
            | fivehost.com |

    Scenario Outline: Try to delete a non existing resource or a path to receive a 404 error
        Given set connection and headers for ip "127.0.0.1" port "8183" and domain "<host>"
        When send a "DELETE" request to "/party_static.webp" using set up domain and headers and status code "404"
        Then send a "DELETE" request to "/" using set up domain and headers and status code "404"

        Examples:
            | host         |
            | localhost    |
            | fivehost.com |


