var rebooted  = true;
var lockUI    = false;
var uploading = false;
var upgrading = false;
var elapsed   = 0;
var timer;
var count     = 60;

function detectBrowser() {
    var sAgent   = navigator.userAgent.toLowerCase();
    this.isIE    = (sAgent.indexOf("msie")     != -1); //IE6.0-7
    this.isFF    = (sAgent.indexOf("firefox")  != -1); //firefox
    this.isSa    = (sAgent.indexOf("safari")   != -1); //safari
    this.isOp    = (sAgent.indexOf("opera")    != -1); //opera
    this.isNN    = (sAgent.indexOf("netscape") != -1); //netscape
    this.isGc    = (sAgent.indexOf("chrome")   != -1); //netscape
    this.isMa    = this.isIE;                          //maxthon
    this.isOther = (!this.isIE && !this.isFF && !this.isSa && !this.isOp && !this.isNN && !this.isSa && !this.isGc); //unknown Browser
}

function notSupportOldIE() {
    if (document.getElementById("fUpgrade"))
        document.getElementById("fUpgrade").disabled  = true;

    if (document.getElementById("rebootBtn"))
        document.getElementById("rebootBtn").disabled = true;

    if (document.getElementById("updatebtn"))
        document.getElementById("updatebtn").disabled = true;
}

function checkUploadComplete() {
    if (uploading == false)
        return;

    if (elapsed >= 200000) {
        alert("Timeout!");
    } else {
        setTimeout(checkUploadComplete, 2000);
        elapsed += 2000;
    }
}

function startUpload() {
    if(lockUI)
        return;

    var filename = document.getElementById("fUpgrade").filename.value;
    if (filename != '' && !lockUI) {
        if (!uploading && (confirm("Upgrade firmware?")==true)) {

            lockUI = true;
            var iFrame = document.getElementById("iframeupload");
            var iFrameDoc = iFrame.contentDocument ? iFrame.contentDocument : iFrame.contentWindow.document;

            uploading = true;
            elapsed = 0;

            checkUploadComplete();
        } else {
            alert("User cancels firmware upload.");
            return false;
        }
    } else {
        alert("Please select a firmware file.");
        return false;
    }
}

function uploadComplete() {
    if (!uploading) {
        document.getElementById("fUpgrade").filename.value = "";
        return;
    }

    elapsed = 0;
    uploading = false;
    try {
        var iFrame = document.getElementById("iframeupload");
        var iFrameDoc = iFrame.contentDocument ? iFrame.contentDocument : iFrame.contentWindow.document;
        alert("Firmware uploaded successfully.");

        clearTimeout(timer);

        var nocache = "&nocache=" + Math.random() * 1000000;
        var hostStr = "http://" + location.host + "/dev/info.cgi?action=upgrade" + nocache;
        ajaxSendRequest(hostStr, true, onRcvDataUpgrade);
    } catch (e) {
        alert("Upload Fail: " + e);
    }
}

//check browser fot creating XMLHttpRequest object.
function createAJAX() {
    if (window.XMLHttpRequest) {
        return new XMLHttpRequest();
    } else if (window.ActiveXObject) {
        try {
            return new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
                return new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e2) {
                return null;
            }
        }
    } else {
        return null;
    }
}

function onRcvDataUpgrade() {
    if (ajax.readyState == 4) {
        if (ajax.status == 200) {
            alert("Firmware upgraded successfully, reboot system....");
            rebootSystem();
            document.getElementById("fUpgrade").filename.value = "";
            window.stop ? window.stop() : document.execCommand("Stop");
        } else {
            alert("Upgrade error: " + ajax.responseText + ", status: " + ajax.status + ", readyState: " + ajax.readyState);
        }
    }
}

function ajaxSendRequest(uri, isRcved, callback) {
    ajax = createAJAX();
    if (!ajax) {
        alert("Create ajax object error");
        return 0;
    }

    if(isRcved)
        ajax.onreadystatechange = callback;

    ajax.open("GET", uri, true);
    ajax.send(null);
}

function doAjaxSendRequestReboot() {
    var nocache = "&nocache=" + Math.random() * 1000000;
    var hostStr = "http://" + location.host + "/dev/info.cgi?action=reboot&reboot=Reboot" + nocache;
    ajaxSendRequest(hostStr, false);
}

function countDown() {
    count--;
    if (count <= 0) {
        window.location.reload(true);
        document.getElementById("timer").innerHTML = "";
        return;
    }
    document.getElementById("timer").innerHTML = "Reload page after " + count + ' seconds';
}

function rebootSystem() {
    if ((confirm("Do you want to reboot the device ?")==true)) {
        setTimeout("doAjaxSendRequestReboot()", 5);
        count = 20;
        var counter = setInterval(countDown, 1000);
        return true;
    } else {
        return false;
    }
}

function ajaxSendRequestRebootCheck() {
    if (lockUI)
        return;

    if ((confirm("Do you want to reboot the device ?")==true)) {
        lockUI = true;
        var nocache = "&nocache=" + Math.random() * 1000000;
        var hostStr = "http://" + location.host + "/dev/info.cgi?action=reboot&reboot=Reboot" + nocache;
        ajaxSendRequest(hostStr, false);

        count = 20;
        var counter = setInterval(timer, 1000);
        return true;
    } else {
        return false;
    }
}


function ajaxSendRequestResetCheck() {
    if(lockUI)
        return;

    if ((confirm("Do you want to reset to factory default?")==true)) {
        lockUI = true;
        var nocache = "&nocache=" + Math.random() * 1000000;
        var resetStr = "http://" + location.host + "/dev/info.cgi?action=Reset&Reset=Reset" + nocache;
        ajaxSendRequest(resetStr, false);

        var nocache = "&nocache=" + Math.random() * 1000000;
        var rebootStr = "http://" + location.host + "/dev/info.cgi?action=reboot&reboot=Reboot" + nocache;
        ajaxSendRequest(rebootStr, false);

        count = 20;
        var counter = setInterval(timer, 1000);
        return true;
    } else {
        return false;
    }
}

function checkMacAddrInputKeyUp(id, text) {
    checkMacAddrRegExp(id, text);
}

function checkMacAddrInputChange(id, text) {
    checkMacAddrRegExp(id, text);
}

function checkMacAddrRegExp(id, text) {
    var re = /^[0-9A-Fa-f]+$/gi;
    if (!re.test(text.value)) {
        var str = text.value;
        document.getElementById(id).value = str.substring(0, str.length - 1);
    }
}

function onRcvDataUpdateMacAddr() {
    if (ajax.readyState == 4) {
        if (ajax.status == 200) {
            alert("Update mac address success, reboot system....");
            rebootSystem();
            window.stop ? window.stop() : document.execCommand("Stop");
        } else {
            alert("Update mac address error: " + ajax.responseText + ", status: " + ajax.status + ", readyState: " + ajax.readyState);
        }
    }
}

function onRcvDataUpdateSetting() {
    if (ajax.readyState == 4) {
        if (ajax.status == 200) {
            alert("Update setting success.");
            window.stop ? window.stop() : document.execCommand("Stop");
            window.location.reload(true);
        } else {
            alert("Update setting error: " + ajax.responseText + ", status: " + ajax.status + ", readyState: " + ajax.readyState);
        }
    }
}

function checkMacAddrExpression() {
    for(var i = 0; i < 6; i++) {
        var s = document.getElementById("macaddr" + i).value;
        if(s.length > 0 && s.length == 1)
            document.getElementById("macaddr" + i).value = '0' + s;
    }
}

function updateMacAddrClick() {
    if(lockUI)
        return;

    checkMacAddrExpression();

    var mac_array = new Array();
    for(var i = 0; i < 6; i++)
        mac_array[i] = document.getElementById("macaddr" + i).value;

    var exited = false;
    for(var i = 0; i < 6; i++)
        if(mac_array[i].length != 2) {
            alert("Please check " + (i + 1) + " of 6 number in mac address, it is invalid.");
            exited = true;
        }

    if(exited)
        return;

    if (confirm("Do you want to update mac address setting?")==true) {
        lockUI = true;

        var hostStr = "http://" + location.host + "/dev/info.cgi?action=macaddr";
        var str = "";
        for(var i = 0; i < 6; i++)
            str += "&macaddr" + i + "=" + mac_array[i];

        var nocache = "&nocache=" + Math.random() * 1000000;
        hostStr += str + nocache;
        ajaxSendRequest(hostStr, true, onRcvDataUpdateMacAddr);
        return true;
    } else {
        return false;
    }

}

function checkNumber(id, text) {
    var re = /^[0-9]+$/;
    if (!re.test(text.value)) {
        var str = text.value;
        var regex = /\D+|\s/gi;
        str = str.replace(regex, "");
        document.getElementById(id).value = str;
    }
}

function checkVolume(id, text) {
    checkNumber(id, text);
    if (text.value > 100) {
        document.getElementById(id).value = 100;
    }
}

function updateCtrlBoardSetting() {
    if(lockUI)
        return;

    var $brightness        = document.getElementById("brightness").value;
    var $screensaver_time  = document.getElementById("screensaver_time").value;
    var $keylevel          = document.getElementById("keylevel").value;
    var $play_lev          = document.getElementById("play_lev").value;

    var $screensaver_type = $screensaver_type = document.getElementById("screensaver_type").selectedIndex;
    if ($screensaver_type < 0) {
        $screensaver_type = 0;
    }

    var $lang = "chs";
    switch(document.getElementById("lang").selectedIndex) {
        case 0:
            $lang = "cht";
            break;
        case 1:
            $lang = "chs";
            break;
        case 2:
            $lang = "eng";
            break;
    }

    var $keysound_type = "key1.wav";
    switch(document.getElementById("keysound_type").selectedIndex) {
        case 0:
            $keysound_type = "key1.wav";
            break;
        case 1:
            $keysound_type = "key2.wav";
            break;
        case 2:
            $keysound_type = "key3.wav";
            break;
    }
    var exited = false;

    if (exited)
        return;

    if (confirm("Do you want to update setting?")==true) {
        lockUI = true;

        var nocache = "&nocache=" + Math.random() * 1000000;
        var hostStr = "http://" + location.host + "/dev/info.cgi?action=setting&brightness=" + $brightness + "&screensaver_time=" + $screensaver_time + "&screensaver_type=" + $screensaver_type + "&lang=" + $lang + "&keylevel=" + $keylevel + "&keysound_type=" + $keysound_type + "&play_lev=" + $play_lev + nocache;

        ajaxSendRequest(hostStr, true, onRcvDataUpdateSetting);

        return true;
    } else {
        return false;
    }
}

function clearMarAddr() {
    for(var i = 0; i < 6; i++)
        document.getElementById("macaddr" + i).value = "";
}
