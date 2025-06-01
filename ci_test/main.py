import requests
import subprocess
import time
import os
import unittest
from collections import namedtuple

class SimpleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        os.system("touch ./docs/test.txt")

    def _http_request(self, method, url, body):
        if method == "GET":
            return requests.get(url)
        elif method == "POST":
            return requests.post(url, json=body)
        elif method == "DELETE":
            return requests.delete(url)

    def test_it(self):
        TestCase = namedtuple("TestCase", "name, method, url, body, expected")
        testcases = [
            TestCase(
                name="GET index file",
                method="GET",
                url="http://localhost:8080/index",
                body="",
                expected=200
            ),
            TestCase(
                name="no routing", 
                method="GET",
                url="http://localhost:8080/",
                body="",
                expected=404
            ),
            TestCase(
                name="get different port",
                method="GET", 
                url="http://localhost:8081/different_port/8081/",
                body="",
                expected=200
            ),
            TestCase(
                name="can post method",
                method="POST",
                url="http://localhost:8080/post/",
                body="hoge",
                expected=201
            ),
            TestCase(
                name="can delete method",
                method="DELETE",
                url="http://localhost:8080/delete/test.txt",
                body="",
                expected=200
            ),
            TestCase(
                name="get autoindex",
                method="GET",
                url="http://localhost:8080/root/",
                body="",
                expected=200
            ),
            # TestCase(
            #     name="mistake host name",
            #     method="GET",
            #     url="http://localhost/", body="", expected=400
            # ),

            # added by maitneel
            TestCase(
                name="method not allow",
                method="GET",
                url="http://localhost:8080/post/",
                body="",
                expected=405
            ),
            # TestCase(
            #     name="method not allow",
            #     method="GET",
            #     url="http://localhost:8080/redirect/",
            #     body="",
            #     expected=301
            # ),
            TestCase(
                name="post 10byte",
                method="POST",
                url="http://localhost:8080/post/",
                # crlf があるっぽいので2byte少ない
                body="01234567",
                expected=201
            ),
            TestCase(
                name="post 11byte",
                method="POST",
                url="http://localhost:8080/post/",
                # crlf があるっぽいので2byte少ない
                body="012345678",
                expected=413
            ),
            TestCase(
                name="not found file",
                method="GET",
                url="http://localhost:8080/index/hogehogefugafugapiyopiyo",
                # crlf があるっぽいので2byte少ない
                body="012345678",
                expected=404
            ),
        ]

        for c in testcases:
            actual = self._http_request(c.method, c.url, c.body)
            self.assertEqual(
                actual.status_code,
                c.expected,
                "failed test {} expected {}, actual {}".format(
                    c.name, c.expected, actual.status_code
                ),                
            )

class HostHeaderTest(unittest.TestCase):
    def test_it(self):
        actual = requests.get("http://localhost:8082", headers={"Host": "example.com"})
        self.assertEqual(actual.status_code, 200)

        actual = requests.get("http://localhost:8082", headers={"Host": "localhost"})
        self.assertEqual(actual.status_code, 400)

def main():
    unittest.main()

if __name__ == "__main__":
    main()