/**
 * Psyerns Mods Showreel — Public JavaScript.
 *
 * Progressive enhancement: adds scroll-reveal animations to mod cards
 * using IntersectionObserver. Respects prefers-reduced-motion.
 *
 * @package Psyerns_Mods
 */

( function() {
	'use strict';

	/**
	 * Check whether the user prefers reduced motion.
	 *
	 * @return {boolean} True if reduced motion is preferred.
	 */
	function prefersReducedMotion() {
		return window.matchMedia( '(prefers-reduced-motion: reduce)' ).matches;
	}

	/**
	 * Initialize scroll-reveal animations on mod cards.
	 *
	 * Each .psm-card element receives the class 'psm-card--visible'
	 * when it enters the viewport. CSS handles the actual transition.
	 *
	 * If IntersectionObserver is not supported or the user prefers
	 * reduced motion, all cards are made visible immediately.
	 */
	function initScrollReveal() {
		var cards = document.querySelectorAll( '.psm-card' );

		if ( ! cards.length ) {
			return;
		}

		/*
		 * If reduced motion is preferred or IntersectionObserver is
		 * unavailable, show all cards immediately without animation.
		 */
		if ( prefersReducedMotion() || 'undefined' === typeof IntersectionObserver ) {
			cards.forEach( function( card ) {
				card.classList.add( 'psm-card--visible' );
			} );
			return;
		}

		var observer = new IntersectionObserver(
			function( entries ) {
				entries.forEach( function( entry ) {
					if ( entry.isIntersecting ) {
						entry.target.classList.add( 'psm-card--visible' );
						observer.unobserve( entry.target );
					}
				} );
			},
			{
				rootMargin: '0px 0px -40px 0px',
				threshold: 0.1
			}
		);

		cards.forEach( function( card ) {
			observer.observe( card );
		} );
	}

	/*
	 * Run when the DOM is ready.
	 */
	if ( 'loading' === document.readyState ) {
		document.addEventListener( 'DOMContentLoaded', initScrollReveal );
	} else {
		initScrollReveal();
	}
}() );
