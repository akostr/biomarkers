function RGBtoRGBA(r, g, b){
    if((g == null) && (typeof r === 'string')){
        var hex = r.replace(/^\s*#|\s*$/g, '');
        if(hex.length === 3){
            hex = hex.replace(/(.)/g, '$1$1');
        }
        r = parseInt(hex.substr(0, 2), 16);
        g = parseInt(hex.substr(2, 2), 16);
        b = parseInt(hex.substr(4, 2), 16);
    }

    var min, a = (255 - (min = Math.min(r, g, b))) / 255;

    return {
        r    : r = 0|(r - min) / a,
        g    : g = 0|(g - min) / a,
        b    : b = 0|(b - min) / a,
        a    : a = (0|1000*a)/1000,
        rgba : 'rgba(' + r + ', ' + g + ', ' + b + ', ' + a + ')'
    };
}

console.log(RGBtoRGBA("#DFE5FF")) 
