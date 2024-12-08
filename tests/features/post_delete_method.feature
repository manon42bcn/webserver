Feature: Post and Delete methods

    Scenario Outline: Post and Delete methods details posting and deleting a test resource
        Given set connection and headers for ip "127.0.0.1" port "8183" and domain "<host>"
        And send a "POST" request to "/" using set up domain and headers and status code "201"
            | param_name | value             |
            | file       | party_static.webp |
        # Try to Post the same file to ensure that was created and get a 409 error
        Then send a "POST" request to "/" using set up domain and headers and status code "409"
            | param_name | value             |
            | file       | party_static.webp |
        When I parse html response body
        Then The response body content includes "h2" with content "409 - Conflict"
        # Delete the resource
        When send a "DELETE" request to "/party_static.webp" using set up domain and headers and status code "204"

        Examples:
            | host         |
            | localhost    |
            | fivehost.com |

    Scenario Outline: Send a multipart post request to a server where is allowed
        Given set connection and headers for ip "127.0.0.1" port "8183" and domain "<host>"
        Then send a "POST" request to "/" using set up domain and headers and status code "201"
            | param_name | value             |
            | file1      | party_static.webp |
            | file2      | party.gif         |
        # Try to Post files separately to ensure that were created
        And send a "POST" request to "/" using set up domain and headers and status code "409"
            | param_name | value             |
            | file       | party_static.webp |
        And send a "POST" request to "/" using set up domain and headers and status code "409"
            | param_name | value     |
            | file       | party.gif |
        # try to delete the files using multipart post to check its behaviour
        And send a "POST" request to "/" using set up domain and headers and status code "409"
            | param_name | value             |
            | file1      | party_static.webp |
            | file2      | party.gif         |
        # Delete resources
        And send a "DELETE" request to "/party_static.webp" using set up domain and headers and status code "204"
        And send a "DELETE" request to "/party.gif" using set up domain and headers and status code "204"

        Examples:
            | host         |
            | localhost    |
            | fivehost.com |

    Scenario Outline: Post over request limit gives a 413 error
        Given set connection and headers for ip "127.0.0.1" port "8182" and domain "<host>"
        And send a "POST" request to "/" using set up domain and headers and status code "413"
            | param_name | value             |
            | file       | party_static.webp |
        When I parse html response body
        Then The response body content includes "h2" with content "413 - Payload Too Large"

        Examples:
            | host         |
            | localhost    |
            | fivehost.com |

    Scenario Outline: Send a multipart post over payload limit gives a 413 error
        Given set connection and headers for ip "127.0.0.1" port "8182" and domain "<host>"
        Then send a "POST" request to "/" using set up domain and headers and status code "413"
            | param_name | value             |
            | file1      | party_static.webp |
            | file2      | party.gif         |
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

    Scenario Outline: Try to POST content over a server where is not allowed returns a 405 error
        Given set connection and headers for ip "127.0.0.1" port "8181" and domain "<host>"
        And send a "POST" request to "/" using set up domain and headers and status code "405"
        When I parse html response body
        Then The response body content includes "h2" with content "405 - Method Not Allowed"

        Examples:
            | host      | port | request |
            | one.com   | 8181 | /       |
            | two.com   | 8181 | /       |
            | tree.com  | 8181 | /       |
            | localhost | 8181 | /       |

    Scenario Outline: Try to DELETE content over a server where is not allowed returns a 405 error
        Given set connection and headers for ip "127.0.0.1" port "8181" and domain "<host>"
        And send a "DELETE" request to "/index.html" using set up domain and headers and status code "405"
        When I parse html response body
        Then The response body content includes "h2" with content "405 - Method Not Allowed"

        Examples:
            | host      | port | request |
            | one.com   | 8181 | /       |
            | two.com   | 8181 | /       |
            | tree.com  | 8181 | /       |
            | localhost | 8181 | /       |

    Scenario Outline: Try to POST content with an empty body returns a 400 error
        Given set connection and headers for ip "127.0.0.1" port "8181" and domain "<host>"
        And send a "POST_EMPTY" request to "/index.html" using set up domain and headers and status code "400"
        When I parse html response body
        Then The response body content includes "h2" with content "400 - Bad Request"

        Examples:
            | host      | port | request |
            | one.com   | 8181 | /       |
            | two.com   | 8181 | /       |
            | tree.com  | 8181 | /       |
            | localhost | 8181 | /       |

    Scenario Outline: Post content using chunked body and compare the content of the resource
        Given set connection and headers for ip "127.0.0.1" port "8183" and domain "<host>"
        When send a chunked request to "/" using set up domain and headers with status code "201"
            | param_name | value           |
            | file       | lorem_ipsum.txt |
            | mimetype   | text/plain      |
        And send a "GET" request to "/lorem_ipsum.txt" using set up domain and headers and status code "200"
        And I save html response as "posted_file"
        And open "lorem_ipsum.txt" file and save its content in context with key "sent_file"
            | param_name | value |
            | read_mode  | read  |
        Then send a "POST" request to "/" using set up domain and headers and status code "409"
            | param_name | value           |
            | file       | lorem_ipsum.txt |
        When I parse html response body
        Then The response body content includes "h2" with content "409 - Conflict"
        # Delete the resource
        And send a "DELETE" request to "/lorem_ipsum.txt" using set up domain and headers and status code "204"
        Then the content of "posted_file" and "sent_file" context keys are equal

        Examples:
            | host         |
            | localhost    |
            | fivehost.com |





