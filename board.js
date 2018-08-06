var board = {
    
    image: null,
    ledCoords: [[74, 454], [118, 454], [162, 454], [206, 454], [250, 454], [294, 454], [337, 454], [380, 454],
            null, null, null, null, null, [372, 324]],
    ledDiam: 12,
    
    pinChanged: [],
    
    preload: function() {
        board.image = loadImage('board.png');
    },
    
    setup: function() {
        board.offsX = 380; // todo make automatic
        board.offsY = Math.floor((height - board.image.height) / 2);
        image(board.image, board.offsX, board.offsY);
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
};

