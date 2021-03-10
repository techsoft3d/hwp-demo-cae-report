$(document).ready(function () {
	$('.toggle-all').addClass('focus');
	$('.menu ul li').click(function () {
		$('.menu ul li').removeClass('focus');
		$(this).addClass('focus');
	});
	$('.card').addClass('active');
	$('.toggle-all').click(function () {
		$('.card').addClass('active');
		$( 'div.title h2' ).text( 'All Demos' );
	});
	$('.toggle-new').click(function () {
		$('.card').removeClass('active');
		$('.new').addClass('active');
		$( 'div.title h2' ).text( 'New Demos' );
	});
	$('.toggle-basic').click(function () {
		$('.card').removeClass('active');
		$('.basic').addClass('active');
		$( 'div.title h2' ).text( 'Basic Models' );
	});
	$('.toggle-large').click(function () {
		$('.card').removeClass('active');
		$('.large').addClass('active');
		$( 'div.title h2' ).text( 'Large Models' );
	});
	$('.toggle-bim').click(function () {
		$('.card').removeClass('active');
		$('.bim').addClass('active');
		$( 'div.title h2' ).text( 'BIM / AEC' );
	});
	$('.toggle-plm').click(function () {
		$('.card').removeClass('active');
		$('.plm').addClass('active');
		$( 'div.title h2' ).text( 'PLM / PDM / ERP' );
	});
	$('.toggle-analysis').click(function () {
		$('.card').removeClass('active');
		$('.analysis').addClass('active');
		$( 'div.title h2' ).text( 'Analysis' );
	});
	$('.toggle-advanced').click(function () {
		$('.card').removeClass('active');
		$('.advanced').addClass('active');
		$( 'div.title h2' ).text( 'Advanced Workflows' );
	});
});