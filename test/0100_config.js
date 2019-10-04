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

function test_configure()
{

	const res = Native.configure(12,13,14)
	console.log(['configure result ', res])
	const res2 = Native.configure(12,13,14)
	console.log(['configure result ', res2])
}

test_configure()
