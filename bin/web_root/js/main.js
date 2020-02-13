// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

let track1_settings = {};
let track2_settings = {};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

function save() {
  data = { callback: 'save_settings' };
  $.post('/post',data);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

function update_track1_settings() {
  let [ h_min, h_max ] = $('#track1-hue-range').val().split(';');
  let [ s_min, s_max ] = $('#track1-sat-range').val().split(',');
  let [ v_min, v_max ] = $('#track1-val-range').val().split(',');
  let erosions = $('#track1-erosions').val();
  let dilations = $('#track1-dilations').val();

  let data = {
    callback: 'set_camera1_settings',
    h_min,
    h_max,
    s_min,
    s_max,
    v_min,
    v_max,
    erosions,
    dilations,
    width: 64,
    height: 64,
    x_offset: 16,
    y_offset: 16,
    percent_min: 100,
    percent_max: 0,
  };
  $.post('/post', data);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

function update_track2_settings() {
  let [ h_min, h_max ] = $('#track2-hue-range').val().split(';');
  let [ s_min, s_max ] = $('#track2-sat-range').val().split(',');
  let [ v_min, v_max ] = $('#track2-val-range').val().split(',');
  let erosions = $('#track2-erosions').val();
  let dilations = $('#track2-dilations').val();

  let data = {
    callback: 'set_camera2_settings',
    h_min,
    h_max,
    s_min,
    s_max,
    v_min,
    v_max,
    erosions,
    dilations,
    width: 64,
    height: 64,
    x_offset: 16,
    y_offset: 16,
    percent_min: 100,
    percent_max: 0,    
  };
  $.post('/post', data);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

$(document).ready(function() {
  $.get('/get/camera1_settings', function(data, status) {
    if (status === 'success') {
      track1_settings = JSON.parse(data);
      $('#track1-hue-range').val(`${track1_settings.h_min};${track1_settings.h_max}`).trigger('change');
      $('#track1-sat-range').jRange('setValue',`${track1_settings.s_min},${track1_settings.s_max}`);
      $('#track1-val-range').jRange('setValue',`${track1_settings.v_min},${track1_settings.v_max}`);
      $('#track1-erosions').val(track1_settings.erosions);
      $('#track1-dilations').val(track1_settings.dilations);            
    }
  });

  $.get('/get/camera2_settings', function(data, status) {
    if (status === 'success') {
      track2_settings = JSON.parse(data);
      $('#track2-hue-range').val(`${track2_settings.h_min};${track2_settings.h_max}`).trigger('change');
      $('#track2-sat-range').jRange('setValue',`${track2_settings.s_min},${track2_settings.s_max}`);
      $('#track2-val-range').jRange('setValue',`${track2_settings.v_min},${track2_settings.v_max}`);
      $('#track2-erosions').val(track2_settings.erosions);
      $('#track2-dilations').val(track2_settings.dilations);            
    }
  });

  $('input.nice-number').niceNumber();
  $('input.circle-range-select').lcnCircleRangeSelect();
  $('input.jquery-range').jRange({
    from: 0,
    to: 255,
    step: 1,
    isRange: true,
    theme: 'theme-config',
  });

  $('#track1camera').select2({
    minimumResultsForSearch: Infinity,
    data: [
      { id: 0, text: 'OpenCV ID 0' },
      { id: 1, text: 'OpenCV ID 1' },
    ],
  });

  $('#track1-hue-range').on('change',update_track1_settings);
  $('#track1-sat-range').on('change',update_track1_settings);
  $('#track1-val-range').on('change',update_track1_settings);
  $('#track1-erosions').siblings('button').on('click',update_track1_settings);
  $('#track1-dilations').siblings('button').on('click',update_track1_settings);  

  $('#track2-hue-range').on('change',update_track2_settings);
  $('#track2-sat-range').on('change',update_track2_settings);
  $('#track2-val-range').on('change',update_track2_settings);
  $('#track2-erosions').siblings('button').on('click',update_track2_settings);
  $('#track2-dilations').siblings('button').on('click',update_track2_settings);  

  window.setInterval( function() {
    $.get('/get/camera1_frame',function(data, status) {
      $('#track1-frame').attr('src',`data:image/jpeg;base64,${data}`);
    });
    $.get('/get/camera1_mask',function(data, status) {
      $('#track1-mask').attr('src',`data:image/jpeg;base64,${data}`);
    });

    $.get('/get/camera2_frame',function(data, status) {
      $('#track2-frame').attr('src',`data:image/jpeg;base64,${data}`);
    });
    $.get('/get/camera2_mask',function(data, status) {
      $('#track2-mask').attr('src',`data:image/jpeg;base64,${data}`);
    });


  }, 300);
});

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
