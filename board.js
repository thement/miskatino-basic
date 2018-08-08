var board = {

    image: null,
    ledCoords: [[74, 454], [118, 454], [162, 454], [206, 454], [250, 454], [294, 454], [337, 454], [380, 454],
            null, null, null, null, null, [372, 324]],
    ledDiam: 12,
    btnCoords: [[288, 22], [288, 62], [288, 102]],
    btnWidth: 44,
    btnHeight: 20,
    btnDown: null,
    knobX: 167,
    knobY: 74,
    knobOuter: 44,
    knobInner: 22,
    knobMaxA: Math.PI * 8 / 9,
    knobEngaged: false,

    pinChanged: [],

    preload: function() {
        board.image = loadImage('board.png');
    },

    setup: function() {
        board.offsX = 380; // todo make automatic
        board.offsY = Math.floor((height - board.image.height) / 2);
        image(board.image, board.offsX, board.offsY);
        board.setKnob(board.offsX + board.knobX, board.offsY + board.knobY - board.knobOuter + 1);
    },

    draw: function() {
        if (board.pinChanged.length > 0) {
            for (var p in board.pinChanged) {
                board.drawLed(p, pinState[p]);
            }
            board.pinChanged = [];
        }
    },

    drawLed(p, state) {
        if (board.ledCoords[p] === null) {
            return;
        }
        var x = board.ledCoords[p][0];
        var y = board.ledCoords[p][1];
        var d = board.ledDiam;
        if (state < 1) {
            copy(board.image, x - d - 1, y - d -1, d * 2 + 2, d * 2 + 2,
                board.offsX + x - d - 1, board.offsY + y - d - 1, d * 2 + 2, d * 2 + 2);
        }
        if (state > 0 || state < -1) {
            fill(255, 0, 0);
            if (state < 0) {
                d = Math.floor(d / 4);
            }
            ellipse(board.offsX + x, board.offsY + y, d * 2 + 1, d * 2 + 1);
        }
    },

    mouse: function(x, y, down) {
        if (!down) {
            board.knobEngaged = false;
            board.btnReleased();
            return;
        } else {
            board.setKnob(x, y);
        }
        for (var b = 0; b < 3; b += 1) {
            var bx = board.offsX + board.btnCoords[b][0];
            var by = board.offsY + board.btnCoords[b][1];
            if (x >= bx && y >= by - board.btnHeight && x < bx + board.btnWidth && y < by) {
                board.btnPressed(b, bx, by);
                break;
            }
        }
    },

    btnPressed: function(b, bx, by) {
        var col = get(bx + board.btnWidth / 2, by);
        fill(col[0], col[1], col[2]);
        rect(bx - 1, by - board.btnHeight - 1, board.btnWidth + 2, board.btnHeight + 2);
        fill(0, 0, 0);
        rect(bx - 1, by - 2, board.btnWidth + 2, 3);
        board.btnDown = b;
        pinSignal[b + 10] = 0;
    },

    btnReleased: function() {
        if (board.btnDown === null) {
            return;
        }
        var b = board.btnDown;
        var bx = board.btnCoords[b][0];
        var by = board.btnCoords[b][1];
        copy(board.image, bx - 1, by - board.btnHeight - 1, board.btnWidth + 2, board.btnHeight + 2,
            board.offsX + bx - 1, board.offsY + by - board.btnHeight - 1, board.btnWidth + 2, board.btnHeight + 2);
        pinSignal[b + 10] = null;
        board.btnDown = null;
    },

    setKnob(x, y) {
        var cx = board.offsX + board.knobX;
        var cy = board.offsY + board.knobY;
        var dx = x - cx;
        var dy = y - cy;
        var d = Math.hypot(dx, dy);
        if (d < board.knobInner || d > board.knobOuter) {
            board.knobEngaged = false;
            return;
        }
        board.knobEngaged = true;
        var a = Math.atan2(dx, -dy);
        if (Math.abs(a) > board.knobMaxA) {
            a = Math.sign(a) * board.knobMaxA;
        }
        adcSignal[3] = Math.floor((-a / board.knobMaxA + 1) / 2.001 * 1024);
        copy(board.image, board.knobX - board.knobOuter, board.knobY - board.knobOuter,
                board.knobOuter * 2, board.knobOuter * 2,
                cx - board.knobOuter, cy - board.knobOuter, board.knobOuter * 2, board.knobOuter * 2);
        fill(0, 0, 0);
        var r = (board.knobOuter + board.knobInner) / 2;
        ellipse(cx + Math.sin(a) * r, cy - Math.cos(a) * r, 7, 7);
    },

};

