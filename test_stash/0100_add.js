/*
** © 2018 by Philipp Dunkel, Ben Noordhuis, Elan Shankar
** Licensed under MIT License.
*/

/* jshint node:true */
'use strict';
const { rm, touch, rename } = require('./utils/fs.js');
const { run, sleep } = require('./utils/misc.js');
const process = require('process')
const assert = require('assert')

const Native = require('../addon.node');
const util = require('util')
const path = require('path')

console.log("Here is Native.start", util.inspect(Native.start))
console.log("Here is Native.stop", util.inspect(Native.stop))
console.log("Here is Native.cancel", util.inspect(Native.cancel))
console.log("Here is Native.add", util.inspect(Native.add))


async function doit() 
{
	const cb = (...args) => {
		console.log("got something", args)
	}
	const arg0 = {
		schedule : [
			{ident : 0, interval : 1, repeats : 3, replyobj : { time : Date.now()} },
			{ident : 1, interval : 1, repeats : 5, replyobj : { time : Date.now()} },
			{ident : 2, interval : 1, repeats : 7, replyobj : { time : Date.now()} },
		]
	}
	var expire_count = 0;
	var interval_count = [0,0,0];
	const instance = Native.start(arg0, (event) => {
		console.log("Callback ")
		console.log(["event.type", event.type])
		console.log(["event.ident", event.ident])
		if (event.type === "expire") {
			expire_count++
		} else if (event.type === "interval") {
			interval_count[event.ident]++
		}
	});
	await sleep(20000)
	console.log("About to issue stop")
	Native.stop(instance)
	console.log(["completion test", expire_count, interval_count])
	await(5000)

}

doit()
console.log("we are done")
