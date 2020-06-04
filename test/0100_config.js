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
// demonstrates getting and setting values inside the add on
function testTrue(b, msg) {
	if (!b) {
		console.log(["Failed", msg]);
	}
}
function test_configure() {
	let res = Native.configure(12, 13, 14);
	console.log(["configure result ", res]);
	testTrue(res.ticks_ms === 1000, "Ticks ms");
	testTrue(res.start_delay_ms === 10, "Start delay");
	testTrue(res.stop_delay_ms === 12, "Stop delay");
	res = Native.configure(121, 131, 141);
	console.log(["configure result ", res]);
	testTrue(res.ticks_ms === 12, "Ticks ms");
	testTrue(res.start_delay_ms === 13, "Start delay");
	testTrue(res.stop_delay_ms === 14, "Stop delay");
}

test_configure();
