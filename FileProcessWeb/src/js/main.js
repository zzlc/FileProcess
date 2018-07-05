
function getSelectFile() {
  var fileName = document.getElementById("fileSelect").value;
  document.getElementById("executFileName").value = fileName;
  console.log("Select File:%s", fileName);
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

// 启动分块任务
function startSliceTask(filePath){
  // var filePath = document.getElementById("SelectFileForm.fileName").value;
  if (filePath == null || filePath == undefined || filePath == '') {
    alert("必须指定客户端路径！")
    return;
  }
  var WSH = new ActiveXObject("WScript.Shell");
  WSH.Run(filePath);
}

window.onload = function () {
  // var x = "SelectFile", b;
  // (b = document.createElement("button")).innerHTML = "选择文件 Test";
  // b.setAttribute("onclick", "alert('" + x + "')");
  // document.body.appendChild(b);
}
