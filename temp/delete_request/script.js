(function() {
    'use strict';
    const deleteButton = document.getElementById('delete-button');
    
    deleteButton.onclick = () => {

        let formData = new FormData();    

        formData.append("hoge_key", "hoge_val");
        formData.append("fuga_key", "fuga_val");

        const req = new Request("/index.html", {
            method: "POST",
            // body: element.id.substr('delete-button-'.length,)
            body: formData
        });

        // const req = new Request("/index.html", {
        //     method: "DELETE",
        //     body: 'foo:bar'
        // });
        fetch(req).then((res) => {
            console.log(res);
        })
        location.reload();
    }
    
    alert("loaded!");
})();