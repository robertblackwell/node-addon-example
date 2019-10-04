/*
** © 2018 by Philipp Dunkel, Ben Noordhuis, Elan Shankar
** Licensed under MIT License.
*/

/* jshint node:true */
'use strict';
const { rm, touch, rename } = require('./utils/fs.js');
const { run, sleep } = require('./utils/misc.js');

// const native = require('../fsevents.node');
// const fsevents = require('../fsevents');
const Native = require('../addon.node');
const util = require('util')
const path = require('path')
const p1 = path.dirname(__dirname)
const d1 = path.join(p1, 'src')
const d2 = path.join(p1, 'test')
const f1 = path.join(d1, 'xxcheck.h');
const f2 = path.join(d2, 'xx01_native.js')


console.log("Here is Native.start", util.inspect(Native.start))
console.log("Here is Native.stop", util.inspect(Native.stop))
console.log("Here is Native.cancel", util.inspect(Native.cancel))
console.log("Here is Native.add", util.inspect(Native.add))

async function doit(label, ident) {

	var cancel_flag = false
	var count_after_cancel = 0;
	const cb = (...args) => {
		console.log("got something", args)
	}
	const arg0 = {
		label : label,
		schedule : [
			{ident : ident+0, interval : 1, repeats : 3, replyobj : { time : Date.now()} },
			{ident : ident+1, interval : 1, repeats : 5, replyobj : { time : Date.now()} },
			{ident : ident+2, interval : 1, repeats : 7, replyobj : { time : Date.now()} },
		]
	}
	var tracking = {
		label : label,
		events : [
			{ ident: ident+0, interval : 0, repeats : 0},
			{ ident: ident+1, interval : 0, repeats : 0},
			{ ident: ident+2, interval : 0, repeats : 0},
		]
	}
	var expire_count = 0;
	var interval_count = [0,0,0];
	const instance = Native.start(arg0, (event) => {
		try {
			console.log([label, "Callback "])
			console.log([label, "event.label", event.label])
			console.log([label, "event.type", event.type])
			console.log([label, "event.ident", event.ident])
			if (event.type === "expired") {
				var i = event.ident - ident;
				console.log(['expired', 'i ', i])
				tracking.events[i].repeats += 1;
			} else if (event.type === "interval") {
				var i = event.ident - ident;
				console.log(['interval', 'i ', i])
				tracking.events[i].interval += 1;
			}
		} catch(e) {
			console.log([label, e])
		}
	});
	setTimeout(() => {
		console.log(["Label", "About to issue stop"])
		Native.stop(instance)
		console.log([label, "completion test", util.inspect(tracking)])
	}, 20000);

}

doit('ONE', 100)
doit('TWO', 200)
console.log("we are done")
// setTimeout(()=>{
// 	console.log("in timer cb")
// }, 10000)