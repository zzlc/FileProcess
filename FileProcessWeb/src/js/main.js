
function getSelectFile() {
  var fileName = document.getElementById("fileSelect").value;
  console.log("Select File:%s", fileName);
  if (fileName.length > 0) {
    alert(fileName);
  }
}

function getSelectPath() {
  document.querySelector('#b').addEventListener('change', e => {
    for (let entry of e.target.files) {
      console.log(entry.name, entry.webkitRelativePath);
      document.getElementById("path1").value = entry.webkitRelativePath
      break
    }
  });
}

function getDestPath(){
  var pathName = document.getElementById("pathId").value;
  console.log(pathName)
  alert(pathName)
}

window.onload = function () {
  // var x = "SelectFile", b;
  // (b = document.createElement("button")).innerHTML = "选择文件 Test";
  // b.setAttribute("onclick", "alert('" + x + "')");
  // document.body.appendChild(b);
  var filePath = "path", destPath;
  (destPath = document.createElement("button")).innerHTML = "启动文件处理客户端";
  destPath.setAttribute("onclick", "getDestPath();");
  document.body.appendChild(destPath);
}
