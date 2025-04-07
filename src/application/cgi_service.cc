#include "application/cgi_service.hh"

#include <unistd.h>
#include <sstream>
#include <sys/wait.h>
#include "application/context.hh"
#include "common/constants.hh"
#include "common/enum.hh"

auto CgiService::handle(RequestContext& ctx) -> ResponseContext {
  ResponseContext response;

  int pipefd[2];
  if (pipe(pipefd) == -1) {
    error("pipe failed");
    return innerError(response, "Pipe failed"); 
  }

  int pid = fork();
  if (pid == -1) {
    error("fork failed");
    return innerError(response, "Fork failed");
  }

  // TODO(thebao): 重定向后日志也会丢失, 考虑日志使用stderr输出
  if (pid == 0) {
    // 检查文件描述符状态
    if (pipefd[1] < 0) {
      return innerError(response, "Invalid write-end file descriptor");
    }
    close(pipefd[0]); // Close the read end of the pipe in the child process
    if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
      return innerError(response, "Failed to redirect stdout"); 
    }
    close(pipefd[1]); // Close the original write end of the pipe
    std::string cgiPath = root + ctx.getUri();

    setenv("REQUEST_METHOD", ctx.getMethod().c_str(), 1);
    setenv("REQUEST_URI", ctx.getUri().c_str(), 1);
    setenv("HTTP_HOST", ctx.getHeader("Host").c_str(), 1);
    setenv("HTTP_USER_AGENT", ctx.getHeader("User-Agent").c_str(), 1);

    execl(cgiPath.c_str(), cgiPath.c_str(), nullptr);
    return innerError(response, "Failed to execute CGI script");
  } else {
    close(pipefd[1]); // Close the write end of the pipe in the parent process
    std::ostringstream oss;
    char buffer[4096];
    ssize_t bytesRead {};
    while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
      oss.write(buffer, bytesRead);
    }
    close(pipefd[0]); // Close the read end of the pipe in the parent process
    int status;
    waitpid(pid, &status, 0); // Wait for the child process to finish

    std::string output = oss.str();
    response.setBody(output);
    response.setStatusCode(StatusCode::OK);
    response.setStatusMessage("OK");
    response.addHeader("Content-Type", "text/html");
    response.addHeader("Content-Length", std::to_string(output.size()));
    response.addHeader("Connection", "close");
    return response;
  }
}
