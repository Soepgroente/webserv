function display_name_from_cookies() {
    const cookieStr = document.cookie;
    let key, value
    let first_name = "";
    let last_name = "";

    const cookieArr = cookieStr.split('; ');
    for (let i = 0; i < cookieArr.length; i++) {
        [key, value] = cookieArr[i].split('=');
        if (key == "first_name") {
            first_name = value
        }
        if (key == "last_name") {
            last_name = value
        }
    }

    const full_name = (first_name || last_name)
        ? (`${first_name} ${last_name}`).trim()
        : 'anonymous user';
    document.getElementById("welcome-name").innerHTML = full_name;
}
