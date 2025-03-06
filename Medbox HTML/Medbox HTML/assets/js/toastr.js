"use strict";

$("#toastr-success").click(function() {
    iziToast.success({
        title: 'Logged In',
        message: 'Successfully Login',
        position: 'topRight'
    });
});

$("#toastr-RegSuccess").click(function() {
    iziToast.success({
        title: 'Thanks',
        message: 'You Have Successfully Register',
        position: 'topRight'
    });
});

$("#toastr-warning").click(function() {
    iziToast.warning({
        title: 'Warning',
        message: 'Please check terms before login',
        position: 'topRight'
    });
});

$("#toastr-error").click(function() {
    iziToast.error({
        title: 'Error',
        message: 'Please check all fields',
        position: 'topRight'
    });
});