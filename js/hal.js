

function jsPutc(c) {
    var ta = document.getElementById('output');
    ta.value += String.fromCharCode(c);
}

function onInput() {
    var ti = document.getElementById('input');
    procBas(ti.value);
    ti.value = '';
}

window.onload = function() {
    window.procBas = Module.cwrap('processBasic', 'void', ['string']);
    _initBasic();
};

