var shiftPressed, controlPressed;
var keyCodeA = 'A'.charCodeAt(0), keyCodeZ = 'Z'.charCodeAt(0), keyCodeC = 'C'.charCodeAt(0);
var shifted = {
    49: '!', 50: '@', 51: '#', 52: '$', 53: '%', 54: '^', 55: '&', 56: '*', 57: '(', 48: ')',
    189: '_', 187: '+', 219: '{', 221: '}', 186: ':', 222: '"', 220: '|', 188: '<', 190: '>',191: '?', 192: '~'
};
var unshifted = {
    189: '-', 187: '=', 219: '[', 221: ']', 186: ';', 222: '\'', 220: '\\', 188: ',', 190: '.', 191: '/', 192: '`'
}

var pinState = [];
var pinSignal = [];
var adcSignal = [];

function preload() {
    boardImage = loadImage('board.png');
}

function setup() {
    window.onkeydown = function(e) { 
        return !(e.keyCode == 32 || e.keyCode == 8);
    };
    window.initTimestamp = new Date().getTime();
    window.procBas = Module.cwrap('processBasic', 'void', []);
    window.inputBas = Module.cwrap('inputBasic', 'void', ['number']);
    _initBasic();
    setupPins();
    createCanvas(800, 600);
    shiftPressed = false;
    controlPressed = false;
    setInterval(procBas, 50);
    image(boardImage, 380, (height - boardImage.height) / 2);
    windowResized();
}

function setupPins() {
    for (var i = 0; i < 16; i++) {
        pinState[i] = -1;
        pinSignal[i] = null;
        adcSignal[i] = null;
    }
}

function draw() {
    terminal.draw();
    if (typeof(checked) == 'undefined') {
        console.log(typeof(terminal));
        checked = true;
    }
}

function windowResized() {
    var canvas = document.getElementsByTagName('canvas')[0];
    var scale = min(windowWidth / width, windowHeight / height) * 0.95;
    canvas.style.zoom = '' + scale;
    document.body.style.marginTop = Math.floor((windowHeight - height * scale) / 2) + 'px';
}

function keyPressed() {
    var k = keyCode;
    if (k < 32) {
        switch (k) {
            case ENTER:
            case BACKSPACE:
                onInput(k);
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
                onInput(3);
            } else {
                onInput(k + (shiftPressed ? 0 : 0x20));
            }
        } else {
            if (shiftPressed && (k in shifted)) {
                onInput(shifted[k].charCodeAt(0));
            } else if (!shiftPressed && (k in unshifted)) {
                onInput(unshifted[k].charCodeAt(0));
            } else {
                onInput(k);
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

function timeMs(div) {
    var v = new Date().getTime() - window.initTimestamp;
    if (typeof(div) == 'number' && div > 1) {
        v = Math.floor(v / div);
    }
    return v & 0xFFFFFFFF;
}

function jsPutc(c) {
    if (c == 10) {
        c = 13;
    }
    terminal.chars.push(c);
}

function pinOut(pin, value) {
    if (pin >= 0 && pin < 16) {
        pinState[pin] = value;
    }
}

function pinIn(pin, analog) {
    if (pin < 0 || pin > 15) {
        return 0;
    }
    var ps = pinState[pin];
    if (ps >= 0) {
        return ps ? 1 : 0;
    }
    var pv = pinSignal[pin];
    if (ps < -1 || pv !== null) {
        return pv === 0 ? 0 : 1;
    }
    return Math.floor(Math.random() * 2);
}

function onInput(code) {
    inputBas(code);
    procBas();
}

function storageOp(arg) {
    if (arg < 0) {
        switch (String.fromCharCode(-arg)) {
            case 'W':
                storageTemp = [];
                return 1;
            case 'R':
                storageTemp = localStorage.getItem('prgStore');
                if (storageTemp === null) {
                    return 0;
                }
                storageTemp = JSON.parse(storageTemp);
                storageIdx = 0;
                return 1;
            case 'C':
                localStorage.setItem('prgStore', JSON.stringify(storageTemp));
                return 1;
            case 'G':
                if (typeof(storageTemp) == 'object' && storageIdx < storageTemp.length) {
                    return storageTemp[storageIdx++];
                } else {
                    return 0;
                }
            default:
                return 0;
        }
    } else {
        storageTemp.push(arg);
    }
}

addScript('./terminal.js');
