### Example Siege Command:

```bash
siege -b -t 1M -c 50 http://localhost:8080
```

### Explanation of the command:

- **`-b`**: This flag runs Siege in "benchmarking mode." In this mode, Siege will send requests as fast as possible without waiting between them.
- **`-t 1M`**: This specifies the duration for which the test will run. Here, `1M` means 1 minute. You can adjust this to other time units, like `10S` (10 seconds) or `1H` (1 hour).
- **`-c 50`**: This sets the number of concurrent users (simulated connections) to 50. You can adjust this number based on how much load you want to simulate.
- **`http://localhost:8080`**: The target URL of your server (replace this with your actual server address and port).

### Crash

- Launch the server
- Open this : `http://localhost:8080/file_upload.html`
- Upload a big file (bible.txt [8Mb])
- We crash...