(function() {
    'use strict';
    const deleteButton = document.getElementById('delete-button');
    
    deleteButton.onclick = () => {
        const req = new Request("/index.html", {
            method: "DELETE",
            body: 'foo:bar'
        });
        fetch(req).then((res) => {
            console.log(res);
        })
        location.reload();
    }
    
    alert("loaded!");
})();