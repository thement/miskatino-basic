var terminal = {
    
    charW: null,
    charH: null,
    offsX: 10,
    offsY: 10,
    w: 30,
    h: 25,
    curX: 0,
    curY: 0,
    redraw: true,

    chars: [],
    
    onRedraw: function() {
        textFont('Courier New', 20);
        textStyle(BOLD);
        terminal.charW = Math.ceil(textWidth('A'));
        terminal.charH = Math.ceil(textAscent() + textDescent());
        fill(0, 0, 0);
        rect(terminal.offsX, terminal.offsY, terminal.charW * terminal.w, terminal.charH * terminal.h);
        terminal.redraw = false;
    },

    draw: function() {
        if (terminal.redraw) {
            terminal.onRedraw();
        }
        while (terminal.chars.length > 0) {
            terminal.putc(String.fromCharCode(terminal.chars.shift()));
        }
    },

    putc: function(c) {
        if (c >= ' ') {
            textFont('Courier New', 20);
            textStyle(BOLD);
            color(30, 255, 20);
            fill(30, 255, 20);
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
                rect(terminal.offsX + terminal.curX * terminal.charW, terminal.offsY + terminal.curY * terminal.charH,
                        terminal.charW, terminal.charH);
            }
        }
        if (terminal.curY >= terminal.h) {
            terminal.scroll();
        }
    },

    scroll: function() {
        var w = terminal.charW * terminal.w;
        var h = terminal.charH;
        for (var line = 1; line < terminal.h; line += 1) {
            copy(terminal.offsX, terminal.offsY + line * h, w, h, terminal.offsX, terminal.offsY + (line - 1) * h, w, h);
        }
        fill(0, 0, 0);
        rect(terminal.offsX, terminal.offsY + (terminal.h - 1) * terminal.charH, terminal.w * terminal.charW, terminal.charH);
        terminal.curY -= 1;
    }

};
