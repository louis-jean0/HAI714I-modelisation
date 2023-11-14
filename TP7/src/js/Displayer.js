class Ellipse {
    x;
    y;
    wx;
    wy;

    constructor(x, y, wx, wy) {
        this.x = x;
        this.y = y;
        this.wx = wx;
        this.wy = wy;
    }
}

class Displayer {
    static colors = [
        "#41566E",  // bleu
        "#F44",     // orange-rose
        "#FEE",
        "#C94",     // marron
        "#FFF",
        "#AFF",     // turquoise
        "#ffd800",  // jaune
    ];

    constructor(_htmlElement) {
        this.canvas = document.createElement("canvas");
        this.canvas.setAttribute("width", 800);
        this.canvas.setAttribute("height", 600);
        _htmlElement.appendChild(this.canvas);

        if (this.canvas.getContext) {
            this.g2d = this.canvas.getContext("2d");
        } else {
            this.canvas.write(
                "Votre navigateur ne peut visualiser cette page correctement"
            );
        }

        this.wbg = Displayer.colors[0]; //  window background
        this.obg = Displayer.colors[1]; //  object background
        this.fg = Displayer.colors[2];  //  fg foreground
        this.lc = Displayer.colors[3];  //  line color
        this.lineWidth = 1;
        this.init();
    }

    mousePressed(e) {
        console.log("mousePressed",e) 
    }
    
    getCanvas() {
        return this.canvas;
    }

    init() {
        this.g2d.clearRect(0, 0, this.canvas.width, this.canvas.height);
        this.g2d.fillStyle = this.wbg;
        this.g2d.fillRect(0, 0, this.canvas.width, this.canvas.height);
    }

    drawFramerate(framerate) {
        this.g2d.fillStyle = "black";
        this.g2d.font = "20px Arial";
        this.g2d.fillText(`${framerate.toFixed(2)} FPS`,this.width - 10,this.height - 10);
    }

    //dessin en coordonnées canvas
    drawEllipse(p) {
        let x = p.x,
            y = p.y,
            wx = p.wx,
            wy = p.wy;
        this.g2d.beginPath();
        this.g2d.ellipse(x, y, wx, wy, 0, 0, Math.PI * 2, true);
        this.g2d.fillStyle = this.obg;
        this.g2d.fill();
        this.g2d.strokeStyle = this.fg;
        this.g2d.stroke();
    }

    // dessin ligne en coordonnées canvas
    drawLine(x1, y1, x2, y2) {
        this.g2d.strokeStyle = this.lc;
        this.g2d.beginPath();
        this.g2d.moveTo(x1, y1);
        this.g2d.lineTo(x2, y2);
        this.g2d.stroke();
    }
}
