//let g_couleurs = ["#555", "#555", "#FFF", "#333", "#ffd800", "#C33", "#374"];
let g_couleurs = ["#FEE", "#FFF", "#41566E", "#AFF", "#ffd800", "#C94", "#F44", "#0F0"];

class Colors {
    #attribs = ["fg", "txt", "bg", "lc", "bgSelection", "center", "left", "right", ];
    #saved = new Array();
    #palette = new Array();

    constructor() {
        this.#palette = new Array();
        let c = this.#palette;
        g_couleurs.forEach(x => c.push(x));
        //generation des setters/getters from "enum" of colors (defined in static field for)
        // on crée les setters et getters des attributs possibles: fg, txt...
        // et on fait en sorte que les getters retournent la valeur de la couleur
        // de la palette pour le rang de l'élément choisi
        // par exemple: c.fg -> retourne la valeur de c.palette[0] et c.fg = "#FEE" modifie c.palette[0]
        let that = this;
        this.#attribs.forEach((x, i) => that.createGettersAndSetters(x, i));
    }

    createGettersAndSetters(x, i) {
        Object.defineProperty(this, x, {
            get() { return this.#palette[i]; },
            set(value) { this.#palette[i] = value; }
        });
    }

    restore() {
        this.#palette = new Array();
        let c = this.#palette;
        this.#saved.forEach(x => c.push(x));
    }

    save() {
        this.#saved = new Array();
        let c = this.#saved;
        this.#palette.forEach(x => c.push(x));
    }

    setPalette(tabCol) {
        if (tabCol.length == this.#attribs.length)
            this.#palette = tabCol;
        else
            console.log("warning: cannot change color; need exactly" + this.#attribs.length + " colours to change a new this.palette");
    }
}
