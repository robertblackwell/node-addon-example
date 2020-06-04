# napi\_cfrunloop\_addon

## Overview

This project is an exercise in building a C/C++ naive code Addon for __node__ using the new __N-API__ interface.

There are a number of examples of such Addons available out there but many of them are very simple and do not get to grips with issues that a meaningful Addon might face. So I wanted this example to be a bit more substantial than simply calling a __C__ function. 

In addition I wanted to understand how to use the C++ Object model built on top of N-API and available at [https://github.com/nodejs/node-addon-api](https://github.com/nodejs/node-addon-api).

## The Example Addon

Here is the scenario that this example addresses. 

-	We have a C++ singleton class that provides an asynchronous service, in this case an interval timer service, to its clients.
-	Client code gives the instance of the class a number interval request and repeat counts and the instance counts down the intervals and calls back to the client code after every interval expiration and repeats this until the number of repeats is exhausted. 
-	Multiple clients can ask for interval services simultaneously from the single instance.
-	The class instances runs as a separate thread which is initiated when the class instances is created.
-	The class instances creates a __CFRunLoop__ and runs a __CFTimer__ on that CFRunloop in order to count down the various time intervals. So obviously this project is for Mac/OSX only. At some point I will replace the CFRunloop with a Boost::Asio io_service.
-	The N-API Addon code must:
	-	Create and start the class instance
	-	organize for __javascript__ code to pass interval requests to the class instance and its CFRunLoop
	-	Pass the interval expired callback from the class instance to __javascript__ land.
	-	stop the C++ class instance thread/CFRunloop eventually


## The Native Addon

The native addon provides three functions `start`, `cancel` and `stop` which request interval services and cancel a single request and stop the addon background thread.

## start

The `start` function provides the Addon with an array of interval requests, and a callback function to be called by the Addon when an interval expires.

The `start` functions return value is a __opaque__ value that identifies this `start` calls context within the Addon.

Behind the scenes the first call to `start` creates an instance of the C++ class, starts a background thread and CFRunloop. Subsequence `start` calls reuse the existing C++ class instance.

```js

type Opaque = Any

type IntervalRequest = {
	ident : number,    // unique number to identify this request 
	interval : number, // the number of seconds the interval should last
	repeat : number,   // the number of times each interval should be repeated
	replyobj : Any     // any javascript object
}

type EventType = 'interval'|'expired'|'cancelled'|'cancelled_last'

type IntervalEvent = {	
	type : EventType			
	ident : number,				// identifying which interval the event is associated with
	thread : string 			 // a hex string which identifies the background thread
	timestamp : string			 // a text representation of the time at which the event was created
	replyobj : any				 // the reply object provided with the interval object
}
```

```js

start(requests IntervalRequest[], (event IntervalEvent): void) : Opaque 

```

## cancel

```js
cancel(opaque: Opaque, (event IntervalEvent): void): void
```

The cancel function stops interval processing for a single Opaque. When the cancel is complete two events are propagated to signal this fact:

-	an event of type 'cancelled' or 'cancelled_last' is propagated to the `cancel()` functions callback.

-	an event of type 'cancelled' or 'cancelled_last' is propagated to the `start()` functions callback.

If the Opaque passed to the cancel call is the only one active the event will be `cancelled_last` otherwise it will be `cancelled`.

I chose to pass events to both the `start()` and `cancel()` callbacks as these are asynchronous and only by placing the `cancelled` event in the `start()` functions event stream is it evident that this is the last event for a given Opaque.

## stop

The `stop` function takes a single argument, the opaque instance returned by a `start` function call, and stops all outstanding interval requests associated with the given Opaque, cleans up all __C__/__C++__ resources associated with the Opaque and stops the background thread and CFRunloop.

The `stop()` function must be called exactly once for each call to start otherwise the Addon will be left in an undefined state.

## Usage

```js
const Addon = require('napi_workerthread_addon');

      const arg0 = {
        schedule : [
          {ident : 0, interval : 2,  repeat: 15, replyobj : { time : Date.now()} },
          {ident : 1, interval : 2,  repeat: 17, replyobj : { time : Date.now()} },
          {ident : 2, interval : 2,  repeat: 19, replyobj : { time : Date.now()} },
        ]
      }

const instance = Addon.start(arg0, (event) => {
  console.log(util.inspect(event))
  if ((event.type === "cancelled_last)||(event.type === "cancelled")) {
  	Addon.stop(instance)
  }
}); // To start receiving events

// to cancel events after a period of time and before the schedule is exhausted
setTimeout(() => {
  Addon.cancel(instance, (event) => {
	  console.log(util.inspect(event))
  });
}, 7000);
```
## The Natve Addon - Additional Functions

The native Addon provides two further functions, `config` and `demo` that illustrate additional facets of Addon construction.

`config` demonstrates how to pass values to, and retrieve values from, an Addon.

`demo` illustrates various ways of passing errors/exceptions/Error from the Addon back to `js` land.

## Installation

This project is not yet an npm package so you will need to clone the repo or install using a URL. This is not really a problem as this project is really for demonstration purposes rather than a useable component. 


## Debugging with XCode

This youtube video explains how to derive an xcode project from the `binding.gyp` specification file and how to run and debug a `nodejs` test program using xcode.

[https://www.youtube.com/watch?v=DND2H2-XfAc](https://www.youtube.com/watch?v=DND2H2-XfAc)

The folder `build-copy` has an xcode project file (binding.xcodeproj) which is setup to run `node` and one of the `js` test programs with the main target. To use this copy `build-copy` to `build` and then local the xcode project file.

The command `npm run xcode` or `node-gyp configure -- -f xcode` will replace the `build` folder and recreate the xcode project file inside that folder. But this re-created project file will __not__ have the main target setup to run a test program using `node`.