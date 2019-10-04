# napi\_workerthread\_addon 

## TODO 

- test that all C++ structures are correctly __delete__ ed as the addon closes down
- implement the abort flag
- npm install mocha and write some mocha tests for the addon, dont forget tests for error conditions. 
- remove all CHECK() macro calls and replace with NAPI_ result test and error test macros

This project is sample __napi__ addon that illustrates (and teaches me) how get an addon to work with a 
C++ package that runs one or more background threads using a runloop technology that is not `libuv` (and hence not from the  [Node.js](https://nodejs.org/) thread pool).


## Installation

This project is not yet an npm package so you will need to clone it or install using a URL.


## Overview

The addon provides two functions `start` and `stop` which control the creation and running of a background thread that is unique to each call to `start`.

### start

A call to `start` creates and starts the execution of a background thread and passes it the data structure that is passed as the first argument to `start`. This argument holds a array of objects each of which specifies a repeating interval timer.

```js
{
	ident : number,    // unique number to identify this entry in the array 
	interval : number, // the number of seconds that each interval should last for
	repeat : number,   // the number of times each interval should be repeated
	replyobj : Any     // any javascript object
}
```

The second argument to the `start` function is a callback that takes one argument called an `event`. Its structure is:

```js
{	
	type : 'interval'|'expired', // depending on whether the repeat count has been exhuasted.
	ident : number,				// identifying which interval the event is associated with
	thread : string 			 // a hex string which identifies the background thread
	timestamp : string			 // a text representation of the time at which the event was created
	replyobj : any				 // the reply object provided with the interval object
}
```

The `start` function returns an _opaque_ which is in fact a pointer to an addon data structure which is unique to each call to `start`.  

### stop

The `stop` function takes a single argument, the opaque instance returned by the `start` function. Thats how the addon knows which background thread to stop.

## Usage

```js
const Addon = require('napi_workerthread_addon');

      const arg0 = {
        schedule : [
          {ident : 0, interval : 2,  repeat: 5, replyobj : { time : Date.now()} },
          {ident : 1, interval : 2,  repeat: 5, replyobj : { time : Date.now()} },
          {ident : 2, interval : 2,  repeat: 5, replyobj : { time : Date.now()} },
        ]
      }

const instance = Addon.start(arg0, (event) => {
  console.log(util.inspect(event))
  if(event.type === "expired") {
    Addon.stop(instance) // this event means the schedule has been exhausted and the worker has nothing to do
  }
}); // To start receiving events

// to stop events after a period of time and before the schedule is exhausted
setTimeout(() => {
  Addon.stop(instance); // To end reception of eventss
}, 10000);
```
