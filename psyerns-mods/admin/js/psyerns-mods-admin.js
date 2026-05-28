/**
 * Psyerns Mods Showreel - Admin JavaScript
 *
 * Handles toggle logic for price fields, AJAX Steam data fetching,
 * and cache flush confirmation.
 *
 * @package Psyerns_Mods
 */

(function( $ ) {
	'use strict';

	/**
	 * Toggle a price input field based on a checkbox state.
	 *
	 * @param {jQuery}  $checkbox  The controlling checkbox element.
	 * @param {jQuery}  $priceField The price input to enable/disable.
	 * @param {boolean} disableWhenChecked True to disable price when checked (Kostenlos),
	 *                                     false to disable when unchecked (Repack/Source toggles).
	 */
	function togglePriceField( $checkbox, $priceField, disableWhenChecked ) {
		var isChecked = $checkbox.is( ':checked' );
		var shouldDisable = disableWhenChecked ? isChecked : ! isChecked;

		$priceField.prop( 'disabled', shouldDisable );

		if ( shouldDisable ) {
			$priceField.val( '' );
		}
	}

	/**
	 * Initialize toggle states for all checkbox-price field pairs.
	 */
	function initToggles() {
		var $isFree      = $( '#psm_is_free' );
		var $price        = $( '#psm_price' );
		var $hasRepack    = $( '#psm_has_repack' );
		var $repackPrice  = $( '#psm_repack_price' );
		var $hasSource    = $( '#psm_has_source' );
		var $sourcePrice  = $( '#psm_source_price' );

		// Set initial states based on current checkbox values.
		togglePriceField( $isFree, $price, true );
		togglePriceField( $hasRepack, $repackPrice, false );
		togglePriceField( $hasSource, $sourcePrice, false );

		// Kostenlos checkbox: disable price when checked.
		$isFree.on( 'change', function() {
			togglePriceField( $( this ), $price, true );
		});

		// Repack toggle: disable repack price when unchecked.
		$hasRepack.on( 'change', function() {
			togglePriceField( $( this ), $repackPrice, false );
		});

		// Source toggle: disable source price when unchecked.
		$hasSource.on( 'change', function() {
			togglePriceField( $( this ), $sourcePrice, false );
		});
	}

	/**
	 * Initialize the AJAX Steam data fetch button.
	 */
	function initSteamFetch() {
		var $button   = $( '#psm_fetch_steam' );
		var $urlInput = $( '#psm_steam_url' );
		var $preview  = $( '#psm_steam_preview' );

		if ( 0 === $button.length ) {
			return;
		}

		$button.on( 'click', function( e ) {
			e.preventDefault();

			var steamUrl = $.trim( $urlInput.val() );
			var postId   = $button.data( 'post-id' );
			var btnText  = $button.text();

			if ( ! steamUrl ) {
				$urlInput.focus();
				return;
			}

			// Show loading state.
			$button.prop( 'disabled', true ).text( psmAdmin.i18n.loading );

			$.ajax({
				url:      psmAdmin.ajaxurl,
				type:     'POST',
				dataType: 'json',
				data: {
					action:    'psm_fetch_steam_data',
					steam_url: steamUrl,
					post_id:   postId,
					_ajax_nonce: psmAdmin.nonce
				},
				success: function( response ) {
					if ( response.success && response.data ) {
						var html = '';

						if ( response.data.preview_url ) {
							html += '<img src="' + escapeHtml( response.data.preview_url ) + '"';
							html += ' alt="' + escapeHtml( response.data.title || '' ) + '"';
							html += ' class="psm-steam-thumbnail" />';
						}

						if ( response.data.title ) {
							html += '<span class="psm-steam-title">' + escapeHtml( response.data.title ) + '</span>';
						}

						if ( html ) {
							$preview.html( html );
						} else {
							$preview.html( '<span class="psm-steam-placeholder">' + escapeHtml( psmAdmin.i18n.noPreview ) + '</span>' );
						}
					} else {
						var msg = ( response.data && response.data.message ) ? response.data.message : psmAdmin.i18n.fetchError;
						$preview.html( '<span class="psm-steam-error">' + escapeHtml( msg ) + '</span>' );
					}
				},
				error: function() {
					$preview.html( '<span class="psm-steam-error">' + escapeHtml( psmAdmin.i18n.fetchError ) + '</span>' );
				},
				complete: function() {
					$button.prop( 'disabled', false ).text( btnText );
				}
			});
		});
	}

	/**
	 * Initialize the cache flush confirmation dialog.
	 */
	function initCacheFlush() {
		var $flushBtn = $( '#psm_flush_cache_btn' );

		if ( 0 === $flushBtn.length ) {
			return;
		}

		$flushBtn.on( 'click', function( e ) {
			if ( ! window.confirm( psmAdmin.i18n.confirmFlush ) ) {
				e.preventDefault();
			}
		});
	}

	/**
	 * Escape HTML entities in a string to prevent XSS.
	 *
	 * @param {string} str Raw string.
	 * @return {string} Escaped string safe for HTML insertion.
	 */
	function escapeHtml( str ) {
		if ( 'string' !== typeof str ) {
			return '';
		}

		var div = document.createElement( 'div' );
		div.appendChild( document.createTextNode( str ) );
		return div.innerHTML;
	}

	/**
	 * Initialize discount toggle on the settings page.
	 */
	function initDiscountToggle() {
		var $discountEnabled = $( '#psm_discount_enabled' );
		var $discountPercent = $( '#psm_discount_percent' );
		var $discountLabel   = $( '#psm_discount_label' );

		if ( 0 === $discountEnabled.length ) {
			return;
		}

		function updateDiscountFields() {
			var enabled = $discountEnabled.is( ':checked' );
			$discountPercent.prop( 'disabled', ! enabled );
			$discountLabel.prop( 'disabled', ! enabled );
		}

		updateDiscountFields();
		$discountEnabled.on( 'change', updateDiscountFields );
	}

	/**
	 * Initialize the shortcode copy button.
	 */
	function initCopyButton() {
		$( '.psm-copy-btn' ).on( 'click', function() {
			var $btn = $( this );
			var text = $btn.data( 'copy' );
			var original = $btn.text();

			if ( navigator.clipboard && navigator.clipboard.writeText ) {
				navigator.clipboard.writeText( text ).then( function() {
					$btn.addClass( 'psm-copy-btn--copied' ).text( 'Kopiert!' );
					setTimeout( function() {
						$btn.removeClass( 'psm-copy-btn--copied' ).text( original );
					}, 2000 );
				});
			} else {
				var $temp = $( '<textarea>' ).val( text ).appendTo( 'body' ).select();
				document.execCommand( 'copy' );
				$temp.remove();
				$btn.addClass( 'psm-copy-btn--copied' ).text( 'Kopiert!' );
				setTimeout( function() {
					$btn.removeClass( 'psm-copy-btn--copied' ).text( original );
				}, 2000 );
			}
		});
	}

	/**
	 * Initialize Discord Leaderboard toggle on the settings page.
	 *
	 * Shows/hides all Discord-specific fields based on the main toggle.
	 */
	function initDiscordToggle() {
		var $discordEnabled = $( '#psm_discord_enabled' );

		if ( 0 === $discordEnabled.length ) {
			return;
		}

		function updateDiscordFields() {
			var enabled = $discordEnabled.is( ':checked' );
			$( '.psm-discord-field' ).toggle( enabled );
		}

		updateDiscordFields();
		$discordEnabled.on( 'change', updateDiscordFields );
	}

	/**
	 * DOM ready: initialize all admin functionality.
	 */
	$( document ).ready( function() {
		initToggles();
		initSteamFetch();
		initCacheFlush();
		initDiscountToggle();
		initDiscordToggle();
		initCopyButton();
	});

})( jQuery );
