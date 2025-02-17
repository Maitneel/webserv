import requests
import subprocess
import time

import unittest
from collections import namedtuple

class Tests(unittest.TestCase):
    def _http_request(self, method, url, body):
        return requests.get(url)

    def test_it(self):
        case = namedtuple("case", "msg method, url, body expected")
        candidates = [
            case(msg="success", method="GET", url="http://localhost:8080/index", body="", expected=200),
        ]
        for c in candidates:
            with self.subTest(msg=c.msg):
                actual = self._http_request(c.method, c.url, c.body)
                self.assertEqual(actual.status_code, c.expected)

def main():
    unittest.main()

if __name__ == "__main__":
    main()