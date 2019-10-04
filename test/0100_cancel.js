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

async function doit() {


		const ar = [d1, d2]
		var cancel_flag = false
		var count_after_cancel = 0;
		// try {
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
				if (cancel_flag) 
					count_after_cancel += 1
				console.log(["count_after_cancel : ", count_after_cancel])
				console.log(["event.type", event.type])
				console.log(["event.ident", event.ident])
				if ((interval_count[1] = 4) && (!cancel_flag)) {
					cancel_flag  = true
					Native.cancel(instance, (event) => {
						console.log(['cancel callback', event]);
					});
				}
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
			// await(50000)
		// } catch(e) {
			
		// 	console.log("catch an error ",e)
		// }
		await(5000)
		// await rm(f1)
		// await rm(f2)
		// await sleep(2000)
		// stopFunction();
		// await sleep(2000)
		// await rm(f1)
		// await rm(f2)
		// setTimeout(async ()=>{
		// 	await touch(f1)
		// 	await touch(f2)
		// 	setTimeout(async()=>{
		// 		await rm(f1)
		// 		await rm(f2)
		// 		stopFunction()
		// 	}, 5000)
		// }, 5000)
		// const result = native.func(ar, cb);

}

doit()
console.log("we are done")
// setTimeout(()=>{
// 	console.log("in timer cb")
// }, 10000)