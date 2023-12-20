const Move = { snake: 0, rigide: 1, orientation: 2, libre: 3 };
const Sens = { backward: 0, forward: 1, mixte: 2 };



class Interactor {
    constructor(p_chain, p_displayer) {
        this.displayer = p_displayer;
        this.chain = p_chain;
        this.state = "idle";
        this.needRefreshing = false;
        this.currentMove = new SnakeMove();

        this.control("mousedown", p_displayer.getCanvas(), this, this.handlePress);
        this.control("mousemove", p_displayer.getCanvas(), this, this.handleMove);
        this.control("mouseup", p_displayer.getCanvas(), this, this.handleUpLeave);
        this.control("mouseleave", p_displayer.getCanvas(), this, this.handleUpLeave);
        this.control("wheel", p_displayer.getCanvas(), this, this.handleWheel);
    }

    control(typeEvt, htmlElement, interactor, callback) {
        htmlElement.addEventListener(typeEvt, function (e) {
            callback.call(interactor, e);
        });
    }

    handleWheel(e) {
        e.preventDefault();
        e.stopImmediatePropagation();
        //todo - rotation 
   }

    handlePress(e) {
        e.preventDefault();
        e.stopImmediatePropagation();
        let selection = this.chain.pick(this.displayer, e);
        let msgOldSelection = this.selection ? this.selection.toString() : this.selection, msgNewSelection = selection ? selection.toString(): undefined;
       //console.log("handlePress - selection = " + msgNewSelection + ", this.selection = " + msgOldSelection + ", state =  " + this.state);
        this.needRefreshing = false;
        this.previousLocation = { x: e.offsetX, y: e.offsetY };
        this.currentMove = new SnakeMove();
        if (this.state === "idle" && selection) {
            this.state = "prepareMove";
            this.changeSelection(selection);
            this.needRefreshing = true;
        } else if (this.state === "selected") {
            if (selection === undefined) {
                this.state = "idle";
                this.changeSelection(undefined);
                this.needRefreshing = true;
            } else {
                this.state = "prepareMove";
                if (selection !== this.selection) {
                    this.changeSelection(selection);
                }
                this.needRefreshing = true;
            }
        }
        if (this.needRefreshing)
            this.chain.draw(this.displayer, undefined, false);
    }

    //fait l'hypothèse que p_selection est différent de this.selection
    changeSelection(p_selection) {
        if (p_selection) console.log("selection change " + p_selection.constructor.name);
        if (this.selection) {
            this.selection.selected = false;
       }
        this.selection = p_selection;
        if (p_selection) this.selection.selected = true;
    }

    handleMove(e) {
        e.preventDefault();
        e.stopImmediatePropagation();
        this.needRefreshing = false;
        let msg = this.selection ? this.selection.toString() : undefined;
 
        if (this.state === "prepareMove") {
            this.state = "move";
        }
        if (this.state === "move") {
            
            this.currentMove.initMove(e, this.previousLocation, this.selection, this.displayer);
            this.previousLocation = { x: e.offsetX, y: e.offsetY };
            this.needRefreshing = true;
        }
        if (this.needRefreshing)
            this.chain.draw(this.displayer, undefined, false);
    }

    handleUpLeave(e) {
        let msg = this.selection ? this.selection.toString() : this.selection;
         e.preventDefault();
        e.stopImmediatePropagation();
        this.needRefreshing = false;

        if (this.state == "move") {
            this.state = "selected";
        } else if (this.state == "prepareMove") {
            this.state = "selected";
        }
        if (this.needRefreshing)
            this.chain.draw(this.displayer, undefined, false);
    }
}

class Displayer {
    static defaultWindowSpace = { w: 600, h: 800 };
    static defaultModelSpace = {
        minX: 0,
        maxX: Displayer.defaultWindowSpace.w,
        minY: 0,
        maxY: Displayer.defaultWindowSpace.h,
    };
    currentTest;

    constructor(_htmlElement) {
        this.canvas = document.createElement("canvas");
        this.canvas.setAttribute("width", Displayer.defaultWindowSpace.w);
        this.canvas.setAttribute("height", Displayer.defaultWindowSpace.h);
        _htmlElement.appendChild(this.canvas);

        if (this.canvas.getContext) {
            this.g2d = this.canvas.getContext("2d");
        } else {
            this.canvas.write(
                "Votre navigateur ne peut visualiser cette page correctement"
            );
        }

        this.lineWidth = 1;
        this.pointSize = 12; //= half width
        this.padding = 4 * this.pointSize;
        this.epsilon = 3 * this.lineWidth;
        this.colors = new Colors();

        //dimensions du display
        this.setDisplaySpace(this.padding, this.padding, this.canvas.width, this.canvas.height);

        //dimensions des données à afficher
        this.setModelSpace(
            Displayer.defaultModelSpace.minX,
            Displayer.defaultModelSpace.minY,
            Displayer.defaultModelSpace.maxX,
            Displayer.defaultModelSpace.maxY
        );

        this.init();
    }

    setTest(_t) {
        this.currentTest = _t;
    }
    getCanvas() {
        return this.canvas;
    }
    getPoint(e) {
        let bb = this.canvas.getBoundingClientRect();
        let that = this;
        if (this.points)
            return this.points.filter((p) =>
                that.intersect(p, { x: e.offsetX, y: e.offsetY })
            );
    }

    setModelSpace(minX, minY, maxX, maxY) {
        //dimensions des données à afficher
        this.minX = minX ?? 0;
        this.maxX = maxX ?? this.canvas.width;
        this.minY = minY ?? 0;
        this.maxY = maxY ?? this.canvas.height;
    }

    setDisplaySpace(x0,y0, w, h) {
        //dimensions du display
        this.cWidth = w;
        this.cHeight = h;
        this.x0 = x0;
        this.y0 = y0;
    }

    setOptions(points, incrementalDrawing) {
        this.points = points ?? this.points;
        this.incrementalDrawing = incrementalDrawing ?? false;
        let pointsNumber = this.points ? this.points.length: 0;
        this.pointSize = pointsNumber < 200 ? 8 : pointsNumber < 2000 ? 2 : 1;
        if (!incrementalDrawing) this.init();
    }

    init() {
        this.g2d.clearRect(0, 0, this.cWidth, this.cHeight);
        this.g2d.fillStyle = this.colors.bg;
        this.g2d.fillRect(0, 0, this.cWidth, this.cHeight);
    }
//transformation des coordonnées modèles en coordonnées canvas
    m2p(p_m) {
        let w = (this.cWidth - 2 * this.x0),
            h = (this.cHeight - 2 * this.y0),
            minX = this.minX,
            minY = this.minY,
            deltax = (this.maxX - minX),
            deltay = (minY - this.maxY);
        
        let alphax = w / deltax, alphay = - h / deltay;
        let alpha = alphax < alphay ? alphax : alphay;

        let p = new Coord2D(
            (p_m.x - minX ) * alpha + this.x0,
            (p_m.y - minY) * (-alpha) + this.cHeight - this.y0,
            p_m.label
        );

        return p;
    }

 //transformation des coordonnées canvas en coordonnées modèle
   p2m(p_p) {
        let w = (this.cWidth - 2 * this.x0),
            h = (this.cHeight - 2 * this.y0),
            minX = this.minX,
            minY = this.minY,
            deltax = (this.maxX - minX),
            deltay = (minY - this.maxY);

        let alphax = w / deltax, alphay = - h / deltay;
        let alpha = alphax < alphay ? alphax : alphay;

        let p = new Coord2D(
            (p_p.x - this.x0) / alpha  + minX,
            (p_p.y - this.cHeight + this.y0) / (-alpha) + minY,
            p_p.label
        );
        return p;
    }

    // mc - { mc.x, mc.y } coordonnées de la souris dans le canvas
    // ce est un élement du modèle affiché dans le canvas
    intersect(ce, mc) {
        let x = ce.getCoordCopy(false).x,
            y = ce.getCoordCopy(false).y,
            r = this.pointSize + this.epsilon;
        let b =
            mc.x <= x + r && mc.x >= x - r && mc.y <= y + r && mc.y >= y - r;

        return b;
    }

    //dessin point donné par ses coordonnées dans le modèle
    mDrawPoint(c) {
        let p = this.m2p(c);
        p.label = c.label;
        this.pDrawPoint(p, false);
    }

    //dessin d'un noeud du modèle
   mDrawNode(n) {
        if (n.cc === undefined)
            n.cc = this.m2p(n.cm);
        n.cc.label = n.cm.label;
        this.pDrawPoint(n.cc, n.selected);
    }

    //dessin point donné avec des coordonnées dans le canvas
    pDrawPoint(p, selected) { 
        this.g2d.beginPath();
        this.g2d.arc(p.x, p.y, this.pointSize, 0, Math.PI * 2, true);
        this.g2d.fillStyle = selected ? this.colors.bgSelection: this.colors.bg;
        this.g2d.fill();
        this.g2d.strokeStyle =  this.colors.fg;
        this.g2d.stroke();
        if (this.pointSize > 6 && p.label) {
            this.g2d.fillStyle = selected ? this.colors.bg : this.colors.txt;
            let dx = p.label.length * 3,
                dy = 3;
            this.g2d.font = "8pt Calibri";
            this.g2d.fillText(p.label, p.x - dx, p.y + dy);
        } else if (!p.label) {
            console.log("drole d'oiseau que ce point la " + p);
        }
    }


    // dessine le vecteur v avec pour origine a
    // coordonnées du modèle
    mDrawVect(a, v) {
        let b = a.plus(v);
        this.mDrawLine(a, b);
    }

    // dessine une edge (flèche) en utilisant 
    // les coordonnées dans le canvas
    drawEdge(edge) {
        let l = this.pointSize;
        let a = edge.from.getCoordCopy(false), b = edge.to.getCoordCopy(false);
        let lv = Coord2D.vecteur(a, b);
        lv.setLength(l);
        
        let f1 = (b.copy());
        f1.moins(lv);
        f1.label = "f1";

        lv.setLength(2 * l);
        let h = b.copy();
        h.moins(lv);
        h.label = "h";
        let u = Coord2D.vecteur(h, f1);
        let v = new Coord2D(- u.y, u.x);
        
        v.setLength(l / 2);
        
        let f2 = h.copy();
        f2.plus(v);
        let f3 = h.copy();
        f3.moins(v);
        this.pDrawLine(edge.from.cc, edge.to.cc);
        this.pDrawLine(f1, f2);
        this.pDrawLine(f1, f3);
    }

    // dessin segment entre points coordonnées modèle
    // coordonnées du modèle traduites en coordonnées canvas
    mDrawLine(p1, p2) {
        this.g2d.strokeStyle = this.colors.lc;
        this.g2d.beginPath();
        this.g2d.moveTo(this.m2p(p1).x, this.m2p(p1).y);
        this.g2d.lineWidth = this.lineWidth;
        this.g2d.lineJoin = "round";
        this.g2d.lineTo(this.m2p(p2).x, this.m2p(p2).y);
        this.g2d.stroke();
    }

    //dessin segment entre points avec coordonnées "canvas"
    pDrawLine(p1, p2) {
        this.g2d.strokeStyle = this.colors.lc;
        this.g2d.beginPath();
        this.g2d.moveTo(p1.x, p1.y);
        this.g2d.lineTo(p2.x, p2.y);
        this.g2d.stroke();
    }


}
