const messageBoardURL = '/cgi/message_board/';

(function (){
    'use strict';

    const deleteButtons = document.getElementsByClassName("delete-button")
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
            setTimeout(location.reload(), (100));
        }
    }

    const submitButton = document.getElementById('submit-button-id');
    const message = document.getElementById('message-input');
    const attachment = document.getElementById('attachment-input');

    submitButton.onclick = () => {
        const postFormData = new FormData();
        postFormData.append('message', message.value);
        postFormData.append('attachment', attachment.files[0]);

        const req = new Request(messageBoardURL, {
            method: "POST",
            body: postFormData
        });
        fetch(req).then((res) => {
            location.reload();
        })
    }
})();
