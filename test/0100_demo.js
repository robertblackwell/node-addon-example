/*
 ** © 2018 by Philipp Dunkel, Ben Noordhuis, Elan Shankar
 ** Licensed under MIT License.
 */

/* jshint node:true */
"use strict";
const { rm, touch, rename } = require("./utils/fs.js");
const { run, sleep } = require("./utils/misc.js");
const process = require("process");
const assert = require("assert");

const Native = require("../addon.node");
const util = require("util");
const path = require("path");
const p1 = path.dirname(__dirname);
const d1 = path.join(p1, "src");
const d2 = path.join(p1, "test");
const f1 = path.join(d1, "xxcheck.h");
const f2 = path.join(d2, "xx01_native.js");

async function test_demo_function() {
	try {
		// no error
		console.log("Pass arg and check return");
		var res = Native.demo("argumentfromjs1");
		console.log(["EXPECTED", "result from configure", util.inspect(res)]);
	} catch (e) {
		console.log(["UNEXPECTED", "test_configure got error", e]);
		assert(false);
	}
	try {
		console.log("Trigger std::string Error");
		var res = Native.demo("argumentfromjs2", 1);
		console.log(["UNEXPECTED", "result from configure", res]);
		assert(false);
	} catch (e) {
		assert(e instanceof Error);
		console.log(["EXPECTED", "test_configure got error", e.message]);
	}
	try {
		console.log("Trigger Napi::Error");
		var res = Native.demo("argumentfromjs2", 2);
		console.log(["UNEXPECTED", "result from configure", res]);
		assert(false);
	} catch (e) {
		assert(e instanceof Error);
		console.log([
			"EXPECTED",
			"test_configure got Napi::Error error",
			e.message
		]);
	}
	try {
		console.log("Trigger std::runtime_error");
		var res = Native.demo("argumentfromjs2", 3);
		console.log(["UNEXPECTED", "result from configure", res]);
		assert(false);
	} catch (e) {
		assert(e instanceof Error);
		console.log(["EXPECTED", "test_configure got runtime_error", e.message]);
	}
	try {
		console.log("Trigger direct js error");
		var res = Native.demo("argumentfromjs2", 4);
		console.log(["UNEXPECTED", "result from configure", res]);
		assert(false);
	} catch (e) {
		assert(e instanceof Error);
		console.log(["EXPECTED", "test_configure got direct js error", e.message]);
	}
}

test_demo_function();
