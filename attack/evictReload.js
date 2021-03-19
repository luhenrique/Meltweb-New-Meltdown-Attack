function evict(){
    
    var sum = Math.random()*256 | 0
    var read = 20000
    var variation = 34

    var f0 = f[0] | 0
    var s0 = s[0] | 0

    var t0 = window.performance.now();

    while (read > 0){
        for (var i = 1; i < 34; i +=1){
            console.log(sum)
            sum += array[f[i]];
            sum += array[s[i]];
            sum += array[f[i+1]];
            sum += array[s[i+1]];
            sum += array[f[i]];
            sum += array[s[i]];
            sum += array[f[i+1]];
            sum += array[s[i+1]];
        }
    }
    

}

