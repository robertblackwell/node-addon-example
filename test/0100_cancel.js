/*
 ** © 2018 by Philipp Dunkel, Ben Noordhuis, Elan Shankar
 ** Licensed under MIT License.
 */

/* jshint node:true */
"use strict"
const {rm, touch, rename} = require("./utils/fs.js")
const {run, sleep} = require("./utils/misc.js")

// const native = require('../fsevents.node');
// const fsevents = require('../fsevents');
const assert = require("assert")
const Native = require("../addon.node")
const util = require("util")
const path = require("path")
const p1 = path.dirname(__dirname)
const d1 = path.join(p1, "src")
const d2 = path.join(p1, "test")
const f1 = path.join(d1, "xxcheck.h")
const f2 = path.join(d2, "xx01_native.js")

console.log("Here is Native.start", util.inspect(Native.start))
console.log("Here is Native.stop", util.inspect(Native.stop))
console.log("Here is Native.cancel", util.inspect(Native.cancel))
console.log("Here is Native.add", util.inspect(Native.add))

// this function executes a single start(), mid way through the event stream it issues
// a cancel() to stop the event stream.
// When event 'cancelled_last' is seen a stop() is issued to close the Addon background
// thread
let cancel_cb_cancelled_event = false
let cancel_cb_cancelled_last_event = false
let start_cb_cancelled_event = false
let start_cb_cancelled_last_event = false
async function one_test(label, stop_interval_secs) {
	// each test only sees one each of a start_cb_... and cancel_cb_...
	let my_cancel_cb_cancelled_event = false
	let my_cancel_cb_cancelled_last_event = false
	let my_start_cb_cancelled_event = false
	let my_start_cb_cancelled_last_event = false

	const ar = [d1, d2]
	var cancel_flag = false
	var count_after_cancel = 0
	const arg0 = {
		label: label,
		schedule: [
			{ident: 0, interval: 1, repeats: 9, replyobj: {time: Date.now()}},
			{ident: 1, interval: 1, repeats: 7, replyobj: {time: Date.now()}},
			{ident: 2, interval: 1, repeats: 11, replyobj: {time: Date.now()}},
		],
	}
	var expire_count = 0
	var interval_count = [0, 0, 0]
	const instance = Native.start(arg0, (event) => {
		console.log([label, "Callback "])
		if (cancel_flag) count_after_cancel += 1
		console.log([label, "count_after_cancel : ", count_after_cancel])
		console.log([label, "event.type", event.type])
		console.log([label, "event.ident", event.ident])

		assert(!my_start_cb_cancelled_event)
		assert(!my_start_cb_cancelled_last_event)

		if (event.type === "expire") {
			expire_count++
		} else if (event.type === "interval") {
			interval_count[event.ident]++
		} else if (event.type === "cancelled") {
			start_cb_cancelled_event = true
			my_start_cb_cancelled_event = true
		} else if (event.type === "cancelled_last") {
			start_cb_cancelled_last_event = true
			my_start_cb_cancelled_last_event = true
		}
		if (interval_count[1] == 4 && !cancel_flag) {
			cancel_flag = true
			console.log(["Cancel issue", "event.ident", event.ident])
			Native.cancel(instance, (event) => {
				console.log([label, "Cancel Callback", event.type])
				console.log([label, "Cancel Callback", event.ident])

				assert(!my_cancel_cb_cancelled_event)
				assert(!my_cancel_cb_cancelled_last_event)

				if (event.type === "cancelled") {
					cancel_cb_cancelled_event = true
					my_cancel_cb_cancelled_event = true
				} else if (event.type === "cancelled_last") {
					cancel_cb_cancelled_last_event = true
					my_cancel_cb_cancelled_last_event = true
				}
			})
		}
	})
	await sleep(stop_interval_secs)
	console.log("About to issue stop")
	Native.stop(instance)
	// because there are two start() calls there should be a cancelled
	// and a cancelled_last event. And both the start() and cancel()
	// callback should see those events
	console.log(["start_cb_cancelled_event: ", start_cb_cancelled_event])
	console.log(["start_cb_cancelled_last_event: ", start_cb_cancelled_last_event])
	console.log(["cancel_cb_cancelled_event: ", cancel_cb_cancelled_event])
	console.log(["cancel_cb_cancelled_last_event: ", cancel_cb_cancelled_last_event])

	assert(start_cb_cancelled_event)
	assert(start_cb_cancelled_last_event)
	assert(cancel_cb_cancelled_event)
	assert(cancel_cb_cancelled_last_event)

	console.log(["completion test", expire_count, interval_count])
}

one_test("ONE", 10000)
one_test("TWO", 12000)
console.log("we are done")
