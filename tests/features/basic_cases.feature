Feature: Basic Request Tests

    Scenario Outline: Use url to send a basic request and gets the right default page
        Given send a request to "http://localhost:<port>/basic_request" and get code status "200"
        When I parse html response body
        Then The response body content includes "h1" with content "This is a styled homepage to test from port <port>"

        Examples:
            | port  |
            | 8080  |
            | 8081  |
            | 9090  |

    Scenario Outline: Different host to the same port resolve different default pages
        Given set connection and headers for ip "127.0.0.1" port "8181" and domain "<host>"
        And send a "GET" request to "/" using set up domain and headers and status code "200"
        When I parse html response body
        Then The response body content includes "h1" with content "This is a styled homepage to test from port 8181 and host <host>"

        Examples:
            | host     |
            | one.com  |
            | two.com  |
            | tree.com |

    Scenario Outline: Extra slashes in the middle of the url are handled correctly
        Given send a request to "http://localhost:<port>//////basic_request" and get code status "200"
        When I parse html response body
        Then The response body content includes "h1" with content "This is a styled homepage to test from port <port>"

        Examples:
            | port  |
            | 8080  |
            | 8081  |
            | 9090  |

    Scenario Outline: Extra slashes at the end of the url are handled correctly
        Given send a request to "http://localhost:<port>/basic_request/////" and get code status "200"
        When I parse html response body
        Then The response body content includes "h1" with content "This is a styled homepage to test from port <port>"

        Examples:
            | port  |
            | 8080  |
            | 8081  |
            | 9090  |

    Scenario Outline: Extra url slashes are handled correctly
        Given send a request to "http://localhost:<port>////basic_request/////" and get code status "200"
        When I parse html response body
        Then The response body content includes "h1" with content "This is a styled homepage to test from port <port>"

        Examples:
            | port  |
            | 8080  |
            | 8081  |
            | 9090  |

    Scenario Outline: Extra slashes using host to the same port are resolved properly
        Given set connection and headers for ip "127.0.0.1" port "8181" and domain "<host>"
        And send a "GET" request to "/////" using set up domain and headers and status code "200"
        When I parse html response body
        Then The response body content includes "h1" with content "This is a styled homepage to test from port 8181 and host <host>"

        Examples:
            | host     |
            | one.com  |
            | two.com  |
            | tree.com |

    Scenario Outline: Using a non existing host is resolved using default one
        Given set connection and headers for ip "127.0.0.1" port "8181" and domain "<host>"
        And send a "GET" request to "/" using set up domain and headers and status code "200"
        When I parse html response body
        Then The response body content includes "h1" with content "This is a styled homepage to test from port 8181 and host <default>"

        Examples:
            | host       | default  |
            | noone.com  | one.com  |
            | notwo.com  | two.com  |
            | notree.com | tree.com |

