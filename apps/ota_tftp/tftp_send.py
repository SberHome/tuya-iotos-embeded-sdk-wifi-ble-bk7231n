import tftpy

server = tftpy.TftpServer(tftproot='.', dyn_file_func=None)
server.listen(listenport=69, timeout=10)