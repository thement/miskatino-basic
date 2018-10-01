var terminal = {
    
    charW: null,
    charH: null,
    fontSize: 20,
    offsX: null,
    offsY: null,
    w: 24,
    h: 24,
    curX: 0,
    curY: 0,

    chars: [],
    
    setup: function() {
        textFont('Courier New', terminal.fontSize);
        textStyle(BOLD);
        terminal.charW = Math.ceil(textWidth('A'));
        terminal.charH = Math.ceil(textAscent() + textDescent());
        terminal.offsX = terminal.charW * 2;
        terminal.offsY = (height - terminal.charH * terminal.h) / 2;
        strokeWeight(terminal.charW * 2);
        noFill();
        stroke(80, 80, 80);
        rect(terminal.offsX - terminal.charW / 2, terminal.offsY - terminal.charW / 2,
                terminal.charW * terminal.w + terminal.charW,
                terminal.charH * terminal.h + terminal.charW, terminal.charW);
        strokeWeight(0);
        fill(0, 0, 0);
        rect(terminal.offsX - 1, terminal.offsY - 1,
                terminal.charW * terminal.w + 2, terminal.charH * terminal.h + 2);
    },

    draw: function() {
        while (terminal.chars.length > 0) {
            terminal.putc(String.fromCharCode(terminal.chars.shift()));
        }
        terminal.drawCursor();
    },
    
    drawCursor: function() {
        terminal.chooseColor(new Date().getTime() % 700 > 350 ? 1 : 0);
        terminal.fillRect(terminal.curX, terminal.curY, 1);
    },

    putc: function(c) {
        terminal.chooseColor(0);
        terminal.fillRect(terminal.curX, terminal.curY, 1);
        if (c >= ' ') {
            textFont('Courier New', terminal.fontSize);
            textStyle(BOLD);
            terminal.chooseColor(1);
            text(c, terminal.offsX + terminal.curX * terminal.charW, terminal.offsY + (terminal.curY + 1) * terminal.charH - textDescent());
            terminal.curX += 1;
            if (terminal.curX >= terminal.w) {
                terminal.curX -= terminal.w;
                terminal.curY += 1;
            }
        } else if (c == '\r') {
            terminal.curX = 0;
            terminal.curY += 1;
        } else if (c == '\b') {
            if (terminal.curX > 0) {
                terminal.curX -= 1;
                fill(0, 0, 0);
                terminal.fillRect(terminal.curX, terminal.curY, 1);
            }
        }
        if (terminal.curY >= terminal.h) {
            terminal.scroll();
        }
    },
    
    mouse: function(x, y, down) {
    },

    scroll: function() {
        var data = get(terminal.offsX, terminal.offsY + terminal.charH,
                terminal.charW * terminal.w, terminal.charH * (terminal.h - 1));
        put(data, terminal.offsX, terminal.offsY);
        fill(0, 0, 0);
        terminal.fillRect(0, terminal.h - 1, terminal.w);
        terminal.curY -= 1;
    },
    
    chooseColor: function(c) {
        if (c) {
            fill(30, 255, 20);
        } else {
            fill(0, 0, 0);
        }
    },
    
    fillRect: function(x, y, len) {
        rect(terminal.offsX + x * terminal.charW, terminal.offsY + y * terminal.charH,
                terminal.charW * len, terminal.charH);
    }

};

