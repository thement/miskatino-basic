var shiftPressed, controlPressed;
var keyCodeA = 'A'.charCodeAt(0), keyCodeZ = 'Z'.charCodeAt(0), keyCodeC = 'C'.charCodeAt(0);
var shifted = {
    49: '!', 50: '@', 51: '#', 52: '$', 53: '%', 54: '^', 55: '&', 56: '*', 57: '(', 48: ')',
    189: '_', 187: '+', 219: '{', 221: '}', 186: ':', 222: '"', 220: '|', 188: '<', 190: '>', 191: '?', 192: '~'
};
var unshifted = {
    189: '-', 187: '=', 219: '[', 221: ']', 186: ';', 222: '\'', 220: '\\', 188: ',', 190: '.', 191: '/', 192: '`'
}

function setup() {
    window.onkeydown = function(e) { 
        return !(e.keyCode == 32);
    };
    createCanvas(800, 600);
    shiftPressed = false;
    controlPressed = false;
}

function draw() {
    terminal.draw();
    if (typeof(checked) == 'undefined') {
        console.log(typeof(terminal));
        checked = true;
    }
}

function keyPressed() {
    var k = keyCode;
    if (k < 32) {
        switch (k) {
            case ENTER:
            case BACKSPACE:
                terminal.chars.push(k);
                break;
            case SHIFT:
                shiftPressed = true;
                break;
            case CONTROL:
                controlPressed = true;
                break;
        }
    } else {
        if (k >= keyCodeA && k <= keyCodeZ) {
            if (k == keyCodeC && controlPressed) {
                terminal.chars.push(3);
            } else {
                terminal.chars.push(k + (shiftPressed ? 0 : 0x20));
            }
        } else {
            if (shiftPressed && (k in shifted)) {
                terminal.chars.push(shifted[k].charCodeAt(0));
            } else if (!shiftPressed && (k in unshifted)) {
                terminal.chars.push(unshifted[k].charCodeAt(0));
            } else {
                terminal.chars.push(k);
            }
        }
    }
}

function keyReleased() {
    switch (keyCode) {
        case SHIFT:
            shiftPressed = false;
            break;
        case CONTROL:
            controlPressed = false;
            break;
    }
}

function addScript(src) {
    var s = document.createElement("script");
    s.type = "text/javascript";
    s.src = src;
    document.head.appendChild(s);
}

addScript('./terminal.js');
