const messageBoardURL = '/cgi/message_board/';

(function (){
    'use strict';

    let deleteButtons = document.getElementsByClassName("delete-button")
    for (let i = 0; i < deleteButtons.length; i++) {
        const element = deleteButtons[i];
        element.onclick = () => {
            const req = new Request(messageBoardURL, {
                method: "DELETE",
                body: element.id.substr('delete-button-'.length,)
            });
            fetch(req).then((res) => {
                console.log(res);
            })
            location.reload();
        }
    }
})();