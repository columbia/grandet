<?php
require_once 'grandet.phar';

var_dump(grandet_put('hello', 'world', ['durability_required' => 1, 'latency_required' => 10, 'bandwidth_required' => 10, 'metadata' => ['meta1' => 'data1', 'meta2' => 'data2']]));
var_dump(grandet_size('hello'));
var_dump(grandet_get('hello'));
var_dump(grandet_get('hello', false, NULL, ['start' => 1]));
var_dump(grandet_get('hello', false, NULL, ['start' => -3]));
var_dump(grandet_get('hello', false, NULL, ['end' => 3]));
var_dump(grandet_get('hello', false, NULL, ['start' => 1, 'end' => 3]));
var_dump(grandet_put_file('hello', '/etc/issue', ['durability_required' => 1, 'latency_required' => 10, 'bandwidth_required' => 10]));
var_dump(grandet_size('hello'));
var_dump(grandet_get('hello', true));
var_dump(grandet_ok(grandet_del('hello')));