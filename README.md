# Futile

Futile is a tiling window manager with none of the following features:

- Config file
- Bar
- Title bars for windows
- Keyboard shortcuts

## Installation

Clone the repo and run `make && sudo make install`

## Configuration

Currently no configuration is possible

## Controlling

All commands are issued to the wm by piping text into the `/tmp/futile` fifo.

`echo 'next' > /tmp/futile`

You can use any method to send these commands but I recommend Simple X Hotkey
Daemon

## Commands

Current commands are:

- `next` - Focusses the next window in the stack
- `prev` - Focusses the previous window in the stack
- `bump` - Sends the focussed window to the top of the stack
- `close` - Closes the focussed window
- `quit` - Quits the window manager
