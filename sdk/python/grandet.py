import grandet_pb2
import socket
import struct


class Grandet(object):
    def __init__(self, host='/tmp/grandet_socket'):
#        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#        self.sock.connect((host, 1481))
#        self.sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.sock.connect(host)

    def _receive(self, size):
        chunks = []
        while size > 0:
            chunk = self.sock.recv(min(size, 4096))
            if chunk == '':
                raise RuntimeError("socket connection broken")
            chunks.append(chunk)
            size -= len(chunk)
        return ''.join(chunks)

    def _query(self, request):
        raw_request = request.SerializeToString()
        request_len = struct.pack('>L', len(raw_request))
        self.sock.sendall(request_len)
        self.sock.sendall(raw_request)
        raw_response_len = self._receive(4)
        response_len = struct.unpack('>L', raw_response_len)[0]
        raw_response = self._receive(response_len)
        response = grandet_pb2.Response()
        response.ParseFromString(raw_response)
        return response

    def put(self, key, value=None, filename=None, durability_required=None,
            latency_required=None, bandwidth_required=None, metadata=None):
        request = grandet_pb2.Request()
        request.type = grandet_pb2.Request.PUT
        request.key = key
        if value is not None:
            request.value.type = grandet_pb2.Value.DATA
            request.value.data = value
        if filename is not None:
            request.value.type = grandet_pb2.Value.FILENAME
            request.value.filename = filename
        if durability_required is not None:
            request.requirements.durability_required = durability_required
        if latency_required is not None:
            request.requirements.latency_required = latency_required
        if bandwidth_required is not None:
            request.requirements.bandwidth_required = bandwidth_required
        if metadata is not None:
            for key in metadata:
                entry = request.requirements.metadata.add()
                entry.meta_key = key.encode('utf-8')
                entry.meta_value = metadata[key].encode('utf-8')
        response = self._query(request)
        return response.status

    def get(self, key):
        request = grandet_pb2.Request()
        request.type = grandet_pb2.Request.GET
        request.key = key
        response = self._query(request)
        return response.value.data

    def delete(self, key):
        request = grandet_pb2.Request()
        request.type = grandet_pb2.Request.DEL
        request.key = key
        response = self._query(request)
        return response.status

    def size(self, key):
        request = grandet_pb2.Request()
        request.type = grandet_pb2.Request.SIZE
        request.key = key
        response = self._query(request)
        return response.value.size
