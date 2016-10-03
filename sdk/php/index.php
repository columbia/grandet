<?php
require_once 'grandet.proto.php';

// $grandet_client = stream_socket_client('tcp://localhost:1481');
$grandet_client = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
socket_connect($grandet_client, gethostbyname('localhost'), 1481);
socket_set_option($grandet_client, SOL_TCP, TCP_NODELAY, 1);

/**
 * @deprecated
 * @internal
 */
function grandet_query_stream($request) {
	global $grandet_client;
	$raw_request = $request->serializeToString();
	$request_len = pack('N', strlen($raw_request));
	fwrite($grandet_client, $request_len);
	fwrite($grandet_client, $raw_request);
	$raw_response_len = stream_get_contents($grandet_client, 4);
	$response_len = unpack('N', $raw_response_len)[1];
	$raw_response = stream_get_contents($grandet_client, $response_len);
	return grandet\Response::parseFromString($raw_response);
}

/**
 * @internal
 */
function grandet_query($request) {
	global $grandet_client;
	$raw_request = $request->serializeToString();
	$request_len = pack('N', strlen($raw_request));
	socket_send($grandet_client, $request_len, 4, 0);
	socket_send($grandet_client, $raw_request, strlen($raw_request), 0);
	socket_recv($grandet_client, $raw_response_len, 4, MSG_WAITALL);
	$response_len = unpack('N', $raw_response_len)[1];
	socket_recv($grandet_client, $raw_response, $response_len, MSG_WAITALL);
	return grandet\Response::parseFromString($raw_response);
}

/**
 * @internal
 */
function grandet_put_helper($key, $value, $requirements) {
	$request = new grandet\Request();
	$request->setType(grandet\Request\Type::PUT);
	$request->setKey($key);
	$proto_value = new grandet\Value();
	if (isset($value['data'])) {
		$proto_value->setType(grandet\Value\Type::DATA);
		$proto_value->setData($value['data']);
	}
	if (isset($value['filename'])) {
		$proto_value->setType(grandet\Value\Type::FILENAME);
		$proto_value->setFilename($value['filename']);
	}
	$request->setValue($proto_value);
	if (!empty($requirements)) {
		$proto_requirements = new grandet\Request\Requirements();
		if (isset($requirements['durability_required'])) {
			$proto_requirements->setDurabilityRequired($requirements['durability_required']);
		}
		if (isset($requirements['latency_required'])) {
			$proto_requirements->setLatencyRequired($requirements['latency_required']);
		}
		if (isset($requirements['bandwidth_required'])) {
			$proto_requirements->setBandwidthRequired($requirements['bandwidth_required']);
		}
		if (isset($requirements['metadata'])) {
			foreach ($requirements['metadata'] as $meta_key => $meta_value) {
				$metadata = new grandet\Request\Requirements\Metadata();
				$metadata->setMetaKey($meta_key);
				$metadata->setMetaValue($meta_value);
				$proto_requirements->appendMetadata($metadata);
			}
		}
		$request->setRequirements($proto_requirements);
	}
	$response = grandet_query($request);
	return $response->getStatus();
}

/**
 * Put an object.
 *
 * @example grandet_put('hello', 'world', ['latency_required' => 1000, 'bandwidth_required' => 1000])
 * @example grandet_put('hello', 'world', ['latency_required' => 1000, 'bandwidth_required' => 1000, 'metadata' => ['meta1' => 'data1', 'meta2' => 'data2']])
 *
 * @param string $key The key of the object.
 * @param string $value The value of the object.
 * @param array $requirements Requirements are given as an array of the following: "latency_required" (latency requirement), "bandwidth_required" (bandwidth requirement), "metadata" (optional, an array of arbitrary key-value pairs).
 * @return int The status code.
 */
function grandet_put($key, $value, $requirements = []) {
	return grandet_put_helper($key, ['data' => $value], $requirements);
}

/**
 * Put a file.
 *
 * @example grandet_put_file('hello', '/path/to/filename', ['latency_required' => 1000, 'bandwidth_required' => 1000])
 * @example grandet_put_file('hello', '/path/to/filename', ['latency_required' => 1000, 'bandwidth_required' => 1000, 'metadata' => ['meta1' => 'data1', 'meta2' => 'data2']])
 *
 * @param string $key The key of the object.
 * @param string $filename The filename.
 * @param array $requirements Requirements are given as an array of the following: "latency_required" (latency requirement), "bandwidth_required" (bandwidth requirement), "metadata" (optional, an array of arbitrary key-value pairs).
 * @return int The status code.
 */
function grandet_put_file($key, $filename, $requirements = []) {
	return grandet_put_helper($key, ['filename' => $filename], $requirements);
}

/**
 * Get an object.
 *
 * @example grandet_get('hello')
 * @example grandet_get('hello', true)
 * @example grandet_get('hello', true, 1451606400)
 * @example grandet_get('hello', false, NULL, ['start' => 1])
 * @example grandet_get('hello', false, NULL, ['start' => -3])
 * @example grandet_get('hello', false, NULL, ['end' => 3])
 * @example grandet_get('hello', false, NULL, ['start' => 1, 'end' => 3])
 *
 * @param string $key The key of the object.
 * @param bool $prefer_url If true, prefer getting the URL of the object instead of its content.
 * @param int $expiration The expiration time of the URL, as the number of seconds since the Epoch. Default is 10 minutes from now.
 * @param array $range The range is given as an array of "start" and "end" (both are inclusive).
 * @return array An array of "status" (the status code) and either "data" (the value of the object) or "url" (the URL of the object).
 */
function grandet_get($key, $prefer_url = false, $expiration = NULL, $range = [], $metadata = []) {
	$request = new grandet\Request();
	$request->setType(grandet\Request\Type::GET);
	$request->setKey($key);
	if ($prefer_url || $range) {
		$requirements = new grandet\Request\Requirements();
		if ($prefer_url) {
			$requirements->setPreferUrl(true);
			if (isset($expiration)) {
				$requirements->setExpiration($expiration);
			} else {
				$requirements->setExpiration(time() + 600);
			}
		}
		if ($range) {
			$proto_range = new grandet\Request\Requirements\Range();
			if (isset($range['start'])) {
				$proto_range->setStart($range['start']);
			}
			if (isset($range['end'])) {
				$proto_range->setEnd($range['end']);
			}
			$requirements->setRange($proto_range);
		}
        if ($metadata) {
            print("META! $metadata\n");
            foreach($metadata as $meta_key => $meta_value) {
				$metadata = new grandet\Request\Requirements\Metadata();
				$metadata->setMetaKey($meta_key);
				$metadata->setMetaValue($meta_value);
				$proto_requirements->appendMetadata($metadata);
            }
        }
		$request->setRequirements($requirements);
	}
	$response = grandet_query($request);
	$result = ['status' => $response->getStatus()];
	$proto_value = $response->getValue();
	switch ($proto_value->getType()) {
	case grandet\Value\Type::DATA:
		$result['data'] = $proto_value->getData();
		break;
	case grandet\Value\Type::URL:
		$result['url'] = $proto_value->getUrl();
		break;
	}
	return $result;
}

/**
 * Delete an object.
 *
 * @example grandet_del('hello')
 *
 * @param string $key The key of the object.
 * @return int The status code.
 */
function grandet_del($key) {
	$request = new grandet\Request();
	$request->setType(grandet\Request\Type::DEL);
	$request->setKey($key);
	$response = grandet_query($request);
	return $response->getStatus();
}

/**
 * Get the size of an object.
 *
 * @example grandet_size('hello')
 *
 * @param string $key The key of the object.
 * @return array An array of "status" (the status code) and "size" (the size of the object).
 */
function grandet_size($key) {
	$request = new grandet\Request();
	$request->setType(grandet\Request\Type::SIZE);
	$request->setKey($key);
	$response = grandet_query($request);
	$result = ['status' => $response->getStatus()];
	$proto_value = $response->getValue();
	if ($proto_value->getType() == grandet\Value\Type::SIZE) {
		$result['size'] = $proto_value->getSize();
	}
	return $result;
}

/**
 * Lock an object.
 *
 * @example grandet_lock('hello')
 *
 * @param string $key The key of the object.
 * @return int The status code.
 */
function grandet_lock($key) {
	$request = new grandet\Request();
	$request->setType(grandet\Request\Type::LOCK);
	$request->setKey($key);
	$response = grandet_query($request);
	return $response->getStatus();
}

/**
 * Unlock an object.
 *
 * @example grandet_unlock('hello')
 *
 * @param string $key The key of the object.
 * @return int The status code.
 */
function grandet_unlock($key) {
	$request = new grandet\Request();
	$request->setType(grandet\Request\Type::UNLOCK);
	$request->setKey($key);
	$response = grandet_query($request);
	return $response->getStatus();
}

/**
 * Check if the response status is OK.
 *
 * @example grandet_ok($status)
 *
 * @param int $status The status code.
 * @return bool Whether the status is OK.
 */
function grandet_ok($status) {
	return $status == grandet\Response\Status::OK;
}
