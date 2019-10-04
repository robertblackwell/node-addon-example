const Native = require('../addon.node');
const { run, sleep } = require('./utils/misc.js');
const path = require('path');
const assert = require('assert');
const async = require('async')

function expected(arg0, ident, event_count) {
	var sum = arg0.schedule[ident].interval * event_count
	return sum
}
/**
* This test demonstrates how to use the Native addon
*/

async function test_one_instance(label, stop_delay, done){
	const cb = (...args) => {
		console.log("got something", args)
	}
	const arg0 = {
		schedule : [
			{ident : 0, interval : 1, repeats : 3, replyobj : { time : (new Date()).getTime()} },
			{ident : 1, interval : 1, repeats : 5, replyobj : { time : (new Date()).getTime()} },
			{ident : 2, interval : 1, repeats : 7, replyobj : { time : (new Date()).getTime()} },
		]
	}
	var counters = [0,0,0];
	const instance = Native.start(arg0, async (event) => {
		console.log([label, "Callback "])
		console.log([label, "event.type", event.type])
		console.log([label, "event.ident", event.ident])
		console.log([label, 'event', event])
		if ((event.type === "expired")||(event.type === "interval")) {
			counters[event.ident] += 1
			// timestamp from addon
			console.log([label, "event.timestamp", event.timestamp])
			console.log([label, "now", (new Date()).getTime()])
			
			// time when we constructed arg0
			console.log([label, "start time  ", arg0.schedule[event.ident].replyobj.time])

			// compute error between actual elapsed time and expected elapsed time
			const expected_elasped = expected(arg0, event.ident, counters[event.ident]) * 1000 // millisecs
			var difference = (((new Date().getTime())) - arg0.schedule[event.ident].replyobj.time) - expected_elasped
			var percent_error = 100.00*(difference / expected_elasped)
			
			console.log([label, "elapsed", ((new Date().getTime())) - arg0.schedule[event.ident].replyobj.time])
			console.log([label, "expected", expected_elasped])
			console.log([label, "% error ", `${percent_error}%`])
			console.log([label, 'ident: ', event.ident, ' counter: ', counters[event.ident]])
			if (Math.abs(percent_error) > 20.0) {

				assert(Math.abs(percent_error) < 20.0);
				console.log([label, "ERROR percent error seems too big", Math.abs(percent_error)])
			
			}
		} else if(event.type == "stopped") {
			console.log([label, "Got stop event"])
		}
	});
	setTimeout(() => {
		console.log("About to issue stop")
		Native.stop(instance)
		console.log(["completion test"])
		done()
	}, stop_delay)

}

function promiseTest (label, stop_delay) {
	return new Promise(function(resolve, reject) {
		test_one_instance(label, stop_delay, ()=>{
			resolve(true)
		})
	})
}

async function test_single(label, stop_delay) {
	await promiseTest(label, stop_delay)
}


function test_two_instances (label, stop_delay1, stop_delay2, done) {
	var counter = 2;
	const cb = () => {
		counter =counter - 1
		if (counter == 0) {
			done()
		}
	}
	test_one_instance(label+"111", stop_delay1, cb)
	test_one_instance(label+"222", stop_delay2, cb)
}
function promiseTestTwo (label, stop_delay1, stop_delay2) {
	return new Promise(function(resolve, reject) {
		test_two_instances(label, stop_delay1, stop_delay2, ()=>{
			resolve(true)
		})
	})
}
async function test_two(label, stop_delay1, stop_delay2) {
	await promiseTestTwo(label, stop_delay1, stop_delay2)
}

async function test_two(label, stop_delay) {
	await promiseTest(label, stop_delay)
}

const ta = [
	function(cb){ test_one_instance("LONG", 10000, cb) },
	function(cb){ test_one_instance("SHORT", 2000, cb) },
	function(cb){ test_one_instance("VERY_SHORT", 20, cb) },
	function(cb){ test_two_instances("TWOXX", 10000, 12000, cb) },
]

async.series(ta)
async.parallel(ta)

// describe('native', function(done) {
// 	it("long single instance  non boundary behaviour start-stop accuracy of timing", function(done) {
// 		doit("", 10000)
// 	})
// 	it("short single instance - stop before schedule is finished", function(done) {
// 		doit("", 2000)
// 	})
// 	it("vshort single instance - stop before schedule is finished", function(done) {
// 		doit("", 20)
// 	})
// 	it("two instance - stop before schedule is finished", function(done) {
// 		var counter = 2;
// 		cb = () => {
// 			counter = counter - 1
// 			if (counter == 0) {
// 				done();
// 			}
// 		}

// 		doit("ONE", cb,	12000)
// 		doit("TWO", cb, 10000)
// 	})
// 	it("five instance - stop before schedule is finished", function(done) {
// 		var counter = 5;
// 		cb = () => {
// 			counter = counter - 1
// 			if (counter == 0) {
// 				done();
// 			}
// 		}

// 		doit("ONE", cb,	12000)
// 		doit("TWO", cb, 10000)
// 		doit("333", cb, 10000)
// 		doit("444", cb, 10000)
// 		doit("555", cb, 10000)
// 	})
// })
