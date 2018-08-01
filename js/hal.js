function millis() {
    return (new Date().getTime() - window.initTimestamp) & 0xFFFFFFFF;
}

function jsPutc(c) {
    var ta = document.getElementById('output');
    ta.value += String.fromCharCode(c);
}

function onInput() {
    var ti = document.getElementById('input');
    inputLine(ti.value + '\n');
    ti.value = '';
}

function inputLine(s) {
    for (var i = 0; i < s.length; i++) {
        inputBas(s.charCodeAt(i));
        procBas();
    }
}

window.onload = function() {
    window.initTimestamp = new Date().getTime();
    window.procBas = Module.cwrap('processBasic', 'void', []);
    window.inputBas = Module.cwrap('inputBasic', 'void', ['number']);
    _initBasic();
    setInterval(procBas, 50);
};

