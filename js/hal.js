function millis() {
    return (new Date().getTime() - window.initTimestamp) & 0xFFFFFFFF;
}

function jsPutc(c) {
    var ta = document.getElementById('output');
    ta.value += String.fromCharCode(c);
    ta.scrollTop = ta.scrollHeight;
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

window.onload = function() {
    window.initTimestamp = new Date().getTime();
    window.procBas = Module.cwrap('processBasic', 'void', []);
    window.inputBas = Module.cwrap('inputBasic', 'void', ['number']);
    _initBasic();
    setInterval(procBas, 50);
};

