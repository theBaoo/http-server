PRD文档
1. 项目背景
项目名称：HTTP/1.1 Web 服务器
项目目标：开发一个功能完备的 HTTP/1.1 Web 服务器，支持基本和高级功能，并能在云服务器上部署和测试。
项目时间：30 天（4 周）
2. 功能需求
2.1 基本功能
支持 GET、HEAD、POST 三种请求方法。
支持 URI 的 "%HEXHEX" 编码解码。
正确生成 HTTP 状态码（如 200 OK、301 Moved Permanently、304 Not Modified、404 Not Found、500 Internal Server Error 等）。
支持 Connection: Keep-Alive 和 Connection: Close 两种连接模式。
实现 多线程 处理并发请求。
支持 异常处理 和错误提示。
提供服务 启动 和 关闭 功能。
记录 请求日志（包括 IP 地址、端口、HTTP 请求行、状态码等）。
2.2 高级功能
支持 HTTPS（基于 SSL/TLS 加密）。
支持 CGI 1.1（动态内容生成）。
支持 Transfer-Encoding: chunked（分块传输编码）。
支持 Content-Encoding: gzip（内容压缩）。
支持 Cookie 管理（包括 Set-Cookie 和 Cookie）。
支持基本 缓存控制（Last-Modified、ETag）。
支持 POST 文件上传（multipart/form-data）。
2.3 非功能需求
性能：支持至少 100 个并发连接。
安全性：HTTPS 实现无明显漏洞，CGI 脚本执行需安全。
可维护性：代码模块化，注释清晰。
可扩展性：易于添加新功能或支持新协议。
3. 验收标准
在云服务器上成功部署，浏览器测试所有功能正常。
HTTP 响应头中 Server 字段设置为你的英文名字。
提供详细的 开发文档 和 测试报告。