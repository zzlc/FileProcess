
function getSelectFile() {
  var fileName = document.getElementById("fileSelect").value;
  document.getElementById("executFileName").value = fileName;
  console.log("Select File:%s", fileName);
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
}

function startSliceTask(sliceCount) {
  // onclick="window.location.href='MediaFileProcess://slice'"
  if (sliceCount <= 1) {
    alert("分块数量至少大于 2");
    console.log("分块数量至少大于 2，" + " current slice count:" + sliceCount);
    return;
  }
  var s = "MediaFileProcess://slice ";
  s += sliceCount;
  s += "end";

  console.log(s);
  window.location.href=s;
}
