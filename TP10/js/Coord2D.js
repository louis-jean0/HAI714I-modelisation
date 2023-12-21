class Coord2D {
   constructor(x, y, label) {
        this.x = x;
        this.y = y;
        this.label = label;
    }
    getRounded(a) {
        if (Number.isInteger(a)) return a;
        else return Math.round(a * 100) / 100;
    }
    getRoundedX() {
        //       return this.x;
        return this.getRounded(this.x);
    }

    getRoundedY() {
        //return this.y;
        return this.getRounded(this.y);
    }

    norme() {
//        console.log("vecteur " + this.label + " = " + this.x + ", " + this.y);
        let squareSum = this.x * this.x + this.y * this.y;
        if (squareSum > 0) return Math.sqrt(squareSum);
        else {
            console.log("warning: in norme@Coord2D a sum of square is negative : " + squareSum);
            return 0;
        }
    }

    setLength(l) {
 //       console.log("old coords = " + this.label + "," + this.x + "," + this.y + " length " + l);
        let n = this.norme();
 //       console.log("norme = " + n);
       if (n != 0) {
            let a = l/n;       
            this.x *= a;
            this.y *= a;
        } else {
           console.log("error in length@Coord2D : attempt to divide by zero");
        }
   }
 
    copy() {
        return new Coord2D(this.x, this.y, this.label);
    }

  //rotation affine dans le plan oxy (orthogonale à l'axe oz)
   rotateZ(alpha, centre) {
        let result = new Coord2D(0, 0, "image+(" + this.label + ")"+"'");
        let cos = Math.cos(alpha),
            sin = Math.sin(alpha);
        result.x =
            cos * this.x -
            sin * this.y -
            cos * centre.x +
            sin * centre.y +
            centre.x;
        result.y =
            sin * this.x +
            cos * this.y -
            sin * centre.x -
            cos * centre.y +
            centre.y;
        return result;
    }

    plus(a) {
        this.x += a.x;
        this.y += a.y;
    }

    moins(a) {
        this.x -= a.x;
        this.y -= a.y;
    }

    static soustraction(a, b) {
        let newLabel = "";
        if (a.label) newLabel += a.label;
        newLabel += "-";
        if (b.label) newLabel += b.label;
        return new Coord2D(a.x - b.x, a.y - b.y, newLabel);
    }

    static addition(a, b) {
        let newLabel = "";
        if (a.label) newLabel += a.label;
        newLabel += "+";
        if (b.label) newLabel += b.label;
        return new Coord2D(a.x + b.x, a.y + b.y, newLabel);
    }

    //a et b sont des points du plan représentés par leurs coordonnées en 2D
    static vecteur(a, b) {
 //       console.log("v = " + a.label + "(" + a.x + "," + a.y + "), " + b.label + "("+b.x + "," + b.y + ")" );
     //   console.log("\n(" + a.label + ", " + b.label+")");
        let resultat = new Coord2D(b.x - a.x, b.y - a.y, a.label);
       /* console.log("v = (" + resultat.x + "," + resultat.y + ")");
        console.log("norme = " + resultat.norme());*/
        return resultat;
    }

    //a et b sont des points du plan représentés par leurs coordonnées en 2D
    static middle(a, b, label) {
        return new Coord2D((b.x + a.x) / 2, (a.y + b.y) / 2, label ?? "m");
    }

    // v1 et v2 sont des vecteurs du plan représentés par leurs coordonnées en 2D
    static determinant(v1, v2) {
        return v1.x * v2.y - v1.y * v2.x;
    }

    toString() {
        return this.label + "(" + this.getRoundedX() + "," + this.getRoundedY() + ")";

    }

    sin() {
        let d = this.norme();
        if (d === 0)
            return 0;
        return this.y / d;
    }

    cos() {
        let d = this.norme();
        if (d === 0)
            return 0;
        return this.x / d;
   }

    // calcul de angle entre 
    //  -> le vecteur de coordonnée this.x, this.y dans[0, 2PI[
    //  -> l'axe ox
    alpha() {
        let sin = this.sin(), cos = this.cos();
        
        if (cos === 0) {
            if (sin > 0) {
                return Math.PI / 2;
            } else if (sin == 0) {
                console.log("Warning : calcul de l'angle du vecteur nul");
                return 0;
            } else {
                return 3 * Math.PI / 2;
            }
        } else if (sin >= 0 && cos > 0) {
            return Math.atan(sin / cos);
        } else if (sin < 0 && cos > 0) {
            return 2 * Math.PI + Math.atan(sin / cos);
        } else { // cos < 0
            return Math.PI + Math.atan(sin / cos);
        }
    }
}
