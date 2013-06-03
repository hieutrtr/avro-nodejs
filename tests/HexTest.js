function parseHexString(str) {
  var result = [];
  while (str.length >= 2) {
    result.push(parseInt(str.substring(0, 2), 16));

    str = str.substring(2, str.length);
  }

  return result;
}

function createHexString(arr) {
    var result = "";
    for (var i = 0; i < arr.length; i++) {
        var str = arr[i].toString(16);

        z = 2 - str.length + 1;
        str = Array(z).join("0") + str;

        result += str;
    }

    return result;
}

b = "78e731027d8fd50ed642340b7c9a63b3";


var c = parseHexString(b);

console.log("Output", c, b.length);

console.log("Input", c);
console.log(createHexString(c), createHexString(c) == b);