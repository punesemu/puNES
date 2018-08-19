#!/bin/sh

if [[ "$(uname)" == "OpenBSD" ]] ; then
	if [[ -z "${AUTOCONF_VERSION:-}" ]] ; then
		export AUTOCONF_VERSION
		AUTOCONF_VERSION="$(ls -1 /usr/local/bin/autoreconf-* | sort | tail -n 1)"
		AUTOCONF_VERSION="${AUTOCONF_VERSION##*-}"
	fi
	if [[ -z "${AUTOMAKE_VERSION:-}" ]] ; then
		export AUTOMAKE_VERSION
		AUTOMAKE_VERSION="$(ls -1 /usr/local/bin/automake-* | sort | tail -n 1)"
		AUTOMAKE_VERSION="${AUTOMAKE_VERSION##*-}"
	fi
fi

autoreconf -vif
