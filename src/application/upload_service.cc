#include "application/upload_service.hh"

#include <cstddef>
#include <fstream>
#include <string_view>

#include "common/enum.hh"

UploadService::UploadService() {
}

auto UploadService::getInstance() -> UploadService& {
  static UploadService instance;
  return instance;
}

auto UploadService::handle(RequestContext& ctx) -> ResponseContext {
  ResponseContext res;
  if (ctx.getMethod() == "GET" || ctx.getMethod() == "HEAD") {
    res.setBody(FileService::getInstance().getFileContent("/upload.html"));
    res.setStatusCode(StatusCode::OK);
    res.setStatusMessage("OK");
    res.addHeader("Content-Type", "text/html");
    res.addHeader("Content-Length", std::to_string(res.getBody()->size()));
    return res;
  }

  // TODO(thebao): 实现文件上传
  // 1. 获取文件名
  // 2. 获取文件内容
  // 3. 保存文件

  // POST请求
  // Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryXyZ
  // ------WebKitFormBoundaryXyZ
  // Content - Disposition : form - data;
  // name     = "file";
  // filename = "hello.txt" Content - Type : text / plain
  std::string ctype = ctx.getHeader("Content-Type");
  auto        pos   = ctype.find("boundary=");
  if (pos == std::string::npos) {
    res.setStatusCode(StatusCode::BAD_REQUEST);
    res.setStatusMessage("Bad Request");
    res.addHeader("Content-Type", "text/plain");
    res.setBody("Invalid Content-Type: excepted boundary");
  }
  // 可能还需要检查; ?
  std::string boundary = ctype.substr(pos + 9);
  log("boundary: {}", boundary);
  log("body: {}", ctx.getBody());

  auto parts = parseMultipartFormData(ctx.getBody(), boundary);
  for (auto& part : parts) {
    if (!part.filename.has_value()) { continue; }
    if (!saveFile(part.filename.value(), part.content)) {
      res.setStatusCode(StatusCode::INTERNAL_SERVER_ERROR);
      res.setStatusMessage("Internal Server Error");
      res.addHeader("Content-Type", "text/plain");
      res.setBody("Failed to save file");
    } else {
      log("Saved file: {}", part.filename.value());
      res.setStatusCode(StatusCode::OK);
      res.setStatusMessage("OK");
      res.addHeader("Content-Type", "text/plain");
      res.setBody("File saved successfully");
    }  
  }

  res.addHeader("Content-Length", std::to_string(res.getBody()->size()));
  res.addHeader("Connection", "close");
  return res;
}

auto saveFile(const std::string& filename, const std::string& content) -> bool // NOLINT
  {
  std::string path = file + filename;
  std::ofstream file(path, std::ios::binary);
  if (!file) {
    return false; // Failed to open file
  }
  file.write(content.data(), static_cast<std::streamsize>(content.size()));
  return true;
}

// 一个part对应一个文件
auto parseMultipartFormData(const std::string_view& body, const std::string& boundary)
    -> std::vector<FormInfo> {
  std::vector<FormInfo> result;
  std::string           delimiter     = "--" + boundary;        // Boundary delimiter
  std::string           end_delimiter = "--" + boundary + "--"; // End boundary marker

  // Find the first boundary
  size_t start = body.find(delimiter);
  if (start == std::string_view::npos) {
    return result; // No parts found
  }
  start += delimiter.size(); // Move past the initial boundary

  while (true) {
    // Find the next boundary or end boundary
    size_t next_delimiter = body.find(delimiter, start);
    size_t end_pos        = body.find(end_delimiter, start);
    if (next_delimiter == std::string_view::npos && end_pos == std::string_view::npos) {
      break; // No more parts to parse
    }

    // Determine the end of the current part
    size_t part_end       = (next_delimiter != std::string_view::npos) ? next_delimiter : end_pos;
    std::string_view part = body.substr(start, part_end - start);

    // Skip leading \r\n after the boundary
    if (part.starts_with("\r\n")) {
      part = part.substr(2);
    }

    FormInfo form_info;

    // Split headers and content
    size_t header_end = part.find("\r\n\r\n");
    if (header_end == std::string_view::npos) {
      continue; // Skip invalid part
    }

    std::string_view headers = part.substr(0, header_end);
    std::string_view content = part.substr(header_end + 4); // Skip \r\n\r\n

    // Parse headers line by line
    size_t pos = 0;
    while (pos < headers.size()) {
      size_t line_end = headers.find("\r\n", pos);
      if (line_end == std::string_view::npos) {
        line_end = headers.size();
      }
      std::string_view header_line = headers.substr(pos, line_end - pos);
      pos                          = line_end + 2; // Move past \r\n

      size_t colon_pos = header_line.find(':');
      if (colon_pos != std::string_view::npos) {
        std::string_view key   = header_line.substr(0, colon_pos);
        std::string_view value = header_line.substr(colon_pos + 1);
        value = value.substr(value.find_first_not_of(" \t")); // Trim leading whitespace

        if (key == "Content-Disposition") {
          // Extract 'name'
          size_t name_pos = value.find("name=\"");
          if (name_pos != std::string_view::npos) {
            name_pos += 6; // Length of "name=\""
            size_t name_end = value.find('\"', name_pos);
            if (name_end != std::string_view::npos) {
              form_info.name = std::string(value.substr(name_pos, name_end - name_pos));
            }
          }

          // Extract 'filename'
          size_t filename_pos = value.find("filename=\"");
          if (filename_pos != std::string_view::npos) {
            filename_pos += 10; // Length of "filename=\""
            size_t filename_end = value.find('\"', filename_pos);
            if (filename_end != std::string_view::npos) {
              form_info.filename =
                  std::string(value.substr(filename_pos, filename_end - filename_pos));
            }
          }
        } else if (key == "Content-Type") {
          form_info.content_type = std::string(value);
        }
      }
    }

    // Clean up content by removing trailing \r\n if present
    if (content.size() >= 2 && content.substr(content.size() - 2) == "\r\n") {
      content = content.substr(0, content.size() - 2);
    }
    form_info.content = content;

    result.push_back(form_info);

    // Move to the next part
    start = part_end + delimiter.size();
    if (part_end == end_pos) {
      break; // Reached the end boundary
    }
  }

  return result;
}