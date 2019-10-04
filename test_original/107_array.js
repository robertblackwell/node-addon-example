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
const Native = require('../fsevents.node');
const util = require('util')
const path = require('path')
const p1 = path.dirname(__dirname)
const d1 = path.join(p1, 'src')
const d2 = path.join(p1, 'test')
const f1 = path.join(d1, 'xxcheck.h');
const f2 = path.join(d2, 'xx01_native.js')

async function doit() {


		const ar = [d1, d2]
		const cb = (...args) => {
			console.log("got something", args)
		}
		try {
			const instance = Native.start({ ident : "thisisanidentityu", strings: ar}, cb);
			console.log('instance: ', util.inspect(instance))
			await sleep(2000)
			await touch(f1)
			await touch(f2)
			await sleep(2000)
		} catch(e) {
			console.log("catch an error ",e)
		}
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