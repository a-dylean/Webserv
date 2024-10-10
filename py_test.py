import requests
import sys

def test_web_server(url):
    try:
        # Send a GET request to the specified URL
        response = requests.get(url)
        # Send a POST request to the specified URL
        # myobj = {'somekey': 'somevalue'}
        # response = requests.post(url, json = myobj)
        # Print the status code of the response
        print(f"Status Code: {response.status_code}")

        # Print the content of the response
        print("Response Content:")
        print(response.text)

    except requests.exceptions.RequestException as e:
        # Print any error that occurs during the request
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    # Check if a URL is provided as a command-line argument
    if len(sys.argv) != 2:
        print("Usage: python3 py_test.py <url, e.g. http://localhost:8080/>")
        sys.exit(1)

    # Get the URL from the command-line argument
    url_to_test = sys.argv[1]

    # Test the web server response
    test_web_server(url_to_test)
